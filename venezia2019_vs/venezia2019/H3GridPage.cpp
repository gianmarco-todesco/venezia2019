#include "H3GridPage.h"

#define _USE_MATH_DEFINES
#include <math.h>
#include <vector>

#include <assert.h>
#include <QKeyEvent>
#include <QWheelEvent>

#include "Gutil.h"
#include "Point3.h"
#include "Polyhedra.h"
#include "H3.h"
#include "H3Grid.h"

#include <QGLShaderProgram>
#include <qmatrix4x4.h>
#include <QVector4D.h>
#include <qvector.h>
#include <qlist.h>
#include <QGLBuffer>
#include <QTIme>

H3GridPage::H3GridPage()
: OpenGLPage()
, m_theta(0)
, m_phi(0)
, m_cameraDistance(10)
, m_rotating(true)
, m_shaderProgram(0)
, m_grid(new H3Grid534())
{
    m_hMatrix.setToIdentity();
}

H3GridPage::~H3GridPage()
{
    delete m_shaderProgram; m_shaderProgram=0;
    delete m_grid; m_grid = 0;
}

void H3GridPage::initializeGL()
{
  glEnable(GL_LIGHTING);
  glEnable(GL_DEPTH_TEST);
  glEnable(GL_LIGHT0);
  // glEnable(GL_LIGHT1);
  glLightModelf(GL_LIGHT_MODEL_TWO_SIDE,1.0);
  glLightModelf(GL_LIGHT_MODEL_LOCAL_VIEWER,1.0);

  GLfloat lcolor[] =  { 0.7f, 0.7f, 0.7f, 1.0f};
  GLfloat lpos[]   = { 5, 7, 10, 1.0f};
  glLightfv(GL_LIGHT0, GL_AMBIENT, lcolor);
  glLightfv(GL_LIGHT0, GL_DIFFUSE, lcolor);
  glLightfv(GL_LIGHT0, GL_SPECULAR, lcolor);
  glLightfv(GL_LIGHT0, GL_POSITION, lpos);

  lpos[0] = -5;
  glLightfv(GL_LIGHT1, GL_AMBIENT, lcolor);
  glLightfv(GL_LIGHT1, GL_DIFFUSE, lcolor);
  glLightfv(GL_LIGHT1, GL_SPECULAR, lcolor);
  glLightfv(GL_LIGHT1, GL_POSITION, lpos);

  m_shaderProgram = loadProgram("h3grid");
  /*
  m_shader0 = loadProgram("rollingSphere");
  m_shaderBG = loadProgram("rollingSphereBg");
  m_texture = Texture::get(":resources/checkboard.png");
  */
 
  QGLFormat glFormat = QGLWidget::format();
  if ( !glFormat.sampleBuffers() )
    qWarning() << "Could not enable sample buffers";

  m_sphere.makeSphere(0.3,10,10);
  
  m_vertexCube.makeCube(0.1,8);
  makeEdgeBox();
  
  build();
  // m_vertexCube.makeBox(0.1,0.2,0.3,5,10,15);
  m_clock.start();
}

void H3GridPage::build()
{
    QMatrix4x4 mat;
    mat.setToIdentity();
    m_edgeMatrices.append(mat);
    m_vertexMatrices.append(mat);
    for(int ix=-10;ix<=10;ix++)
    {
        for(int iy=-10; iy<=10; iy++)
        {
            for(int iz=-10;iz<=10; iz++)
            {
            }
        }
    }
}

void H3GridPage::resizeGL(int width, int height)
{
  double aspect = (float)width/height;
  glViewport(0,0,width,height);
  glMatrixMode(GL_PROJECTION);
  glLoadIdentity();
  gluPerspective(45, aspect, 1.0, 70.0);
  glMatrixMode(GL_MODELVIEW);
  glLoadIdentity();
}


void H3GridPage::makeEdgeBox()
{
    double d = 0.05;
    double edgeLength = m_grid->edgeLength;    
    int n = 10;
    QVector4D base[4] = {
        QVector4D(-d,0,-d,1.0),
        QVector4D( d,0,-d,1.0),
        QVector4D( d,0, d,1.0),
        QVector4D(-d,0, d,1.0)
    };
    QVector<QVector4D> pts;
    for(int i=0; i<=2*n; i++) 
    {
        double hz = 0.5*edgeLength*(1+(double)(i-n)/(double)n);
        QMatrix4x4 tr = H3::KModel::translation(
            QVector3D(0,0,0), 
            QVector3D(0,H3::KModel::getRadius(hz), 0));
        for(int j=0; j<4; j++) pts.append(tr.map(base[j]));
    }
    for(int i=0;i<pts.count();i++) {
        pts[i] *= 1.0/pts[i].w();
    }
    const QVector3D norms[4] = {
        QVector3D(0,0,-1),
        QVector3D(1,0,0),
        QVector3D(0,0,1),
        QVector3D(-1,0,0)
    };
    Mesh &mesh = m_edgeBox;
    for(int j=0; j<4; j++) {
        int j1 = (j+1)%4;
        int k = mesh.m_vCount;
        for(int i=0; i<=2*n; i++) {
            mesh.addVertex(pts[i*4+j].toVector3D(), norms[j]);
            mesh.addVertex(pts[i*4+j1].toVector3D(), norms[j]);
        }
        for(int i=0; i<2*n; i++) {
            mesh.addQuad(k,k+2,k+3,k+1); k+= 2;
        }
    }
    mesh.createBuffers();
}


void H3GridPage::paintGL()
{
    QTime localClock;
    localClock.start();
    

    glClearColor(0,0,0,1);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    drawBackground();

    glPushMatrix();
    glTranslated(0,0,-m_cameraDistance);
    glRotated(m_theta,1,0,0);
    glRotated(m_phi,0,1,0);


    GLfloat specular[] =  { 0.7f, 0.7f, 0.7f, 1.0f};
    glMaterialfv(GL_FRONT_AND_BACK, GL_SPECULAR, specular);
    glMaterialf(GL_FRONT_AND_BACK, GL_SHININESS, 90.0);
    // glMaterialfv(GL_FRONT_AND_BACK, GL_AMBIENT_AND_DIFFUSE, colors[0]);

    draw2();


    glPopMatrix();
  
    qDebug() << localClock.elapsed() << m_clock.restart();
}

void H3GridPage::draw1()
{
    setColor(0.8,0.1,0.1);    
    // drawSphere(QVector3D(-3,0,0), 1);

    setColor(0.8,0.8,0.1);    
    
    QVector<GLushort> indices;
    QVector<GLfloat> buffer;
    int vCount;
    int m = 10, n = 10;
    for(int i=0; i<n; i++)
    {
        double t = (double)i/(double)(n-1);
        for(int j=0;j<m;j++)
        {
            double s = (double)j/(double)(m-1);
            buffer << (2*s-1) * 0.1 << (2*t-1) * 0.1 << 0;
            buffer << 0 << 0 << -1;
            if(i+1<n && j+1<m) {
                indices 
                    << i*m+j << i*m+j+1 << (i+1)*m+j+1
                    << i*m+j << (i+1)*m+j+1 << (i+1)*m+j;
            }
        }
    }

    glEnableClientState(GL_NORMAL_ARRAY);
    glEnableClientState(GL_VERTEX_ARRAY);
    glNormalPointer(GL_FLOAT, sizeof(GL_FLOAT)*6, &buffer[0]+3);
    glVertexPointer(3, GL_FLOAT, sizeof(GL_FLOAT)*6, &buffer[0]);
   
    GLdouble viewArr[16];
    glGetDoublev(GL_MODELVIEW_MATRIX, viewArr);
    QMatrix4x4 view(viewArr);
    QMatrix4x4 mview;
    //  mview.translate(m_pp.x(), 0, m_pp.y());


    m_shaderProgram->bind();
    m_shaderProgram->setUniformValue("view", view);
    m_shaderProgram->setUniformValue("mview", mview);
    m_shaderProgram->setUniformValue("iview", view.inverted());
    m_shaderProgram->setUniformValue("hMatrix", m_hMatrix);

    /*
    m_shaderProgram->setUniformValue("cam_x", (GLfloat)cam.x());
    m_shaderProgram->setUniformValue("cam_y", (GLfloat)cam.y());
    m_shaderProgram->setUniformValue("cam_z", (GLfloat)cam.z());
    m_shader0->setUniformValue("sphere_x", (GLfloat)m_pp.x());
    m_shader0->setUniformValue("sphere_z", (GLfloat)m_pp.y());
    m_shader0->setUniformValue("tex", (GLuint)0);
    */
  

    // glPolygonMode( GL_FRONT_AND_BACK, GL_LINE );

    //m_shaderProgram->setUniformValue("hMatrix", m_hMatrix);
    //glDrawElements(GL_TRIANGLES, indices.size(), GL_UNSIGNED_SHORT, &indices[0]);
    
    QMatrix4x4 ht = H3::KModel::translation(QVector3D(-.1,-.1,0), QVector3D(.1,.1,0));
    //m_shaderProgram->setUniformValue("hMatrix", m_hMatrix * ht);
    //glDrawElements(GL_TRIANGLES, indices.size(), GL_UNSIGNED_SHORT, &indices[0]);


    m_shaderProgram->setUniformValue("hMatrix", m_hMatrix);
    glDrawElements(GL_TRIANGLES, indices.size(), GL_UNSIGNED_SHORT, &indices[0]);

    m_shaderProgram->setUniformValue("hMatrix", m_hMatrix * ht);
    glDrawElements(GL_TRIANGLES, indices.size(), GL_UNSIGNED_SHORT, &indices[0]);

    m_shaderProgram->setUniformValue("hMatrix", m_hMatrix * ht * ht);
    glDrawElements(GL_TRIANGLES, indices.size(), GL_UNSIGNED_SHORT, &indices[0]);


    m_shaderProgram->release();


    glDisableClientState(GL_VERTEX_ARRAY);
    glDisableClientState(GL_NORMAL_ARRAY);
        
    drawBoundary();
}

void H3GridPage::draw2()
{
    glDisable(GL_LIGHTING);
    glColor3d(0.9,0.1,1);

    QMatrix4x4 hMatrix;
    hMatrix.setToIdentity();

    QMatrix4x4 identity;
    identity.setToIdentity();


    hMatrix = m_hMatrix * m_grid->getCornerMatrix();

    QMatrix4x4 edgeMatrix = m_grid->getEdgeMatrix();
    drawDodWf(hMatrix);

    /*
    drawDodWf(edgeMatrix * hMatrix);

    drawBoxWf(identity, 0.1,0.1,0.1);
    drawBoxWf(edgeMatrix, 0.1,0.1,0.1);
    drawBoxWf(edgeMatrix * edgeMatrix, 0.1,0.1,0.1);

    drawEdgeWf(identity);
    drawEdgeWf(m_grid->dirMatrices[0]);
    drawEdgeWf(m_grid->dirMatrices[4]);
    */
    drawEdgeWf(m_hMatrix * m_grid->dirMatrices[0]);
    

    glEnable(GL_LIGHTING);
    drawBoundary();


    // ----- 
    GLdouble viewArr[16];
    glGetDoublev(GL_MODELVIEW_MATRIX, viewArr);
    QMatrix4x4 view(viewArr);
    QMatrix4x4 mview;
    m_shaderProgram->bind();
    m_shaderProgram->setUniformValue("view", view);
    m_shaderProgram->setUniformValue("mview", mview);
    m_shaderProgram->setUniformValue("iview", view.inverted());
    m_shaderProgram->setUniformValue("hMatrix", m_hMatrix);
    
    glEnableClientState(GL_NORMAL_ARRAY);
    glEnableClientState(GL_VERTEX_ARRAY);
    glEnable(GL_CULL_FACE);
    
    // vertices
    m_vertexCube.bind();    
    for(int i=0;i<m_vertexMatrices.count();i++) {
        m_shaderProgram->setUniformValue("hMatrix", m_hMatrix * m_vertexMatrices[i] );
        m_vertexCube.draw();
    }
    m_vertexCube.release();

    // edges
    m_edgeBox.bind();    
    for(int i=0;i<m_edgeMatrices.count();i++) {
        m_shaderProgram->setUniformValue("hMatrix", m_hMatrix * m_edgeMatrices[i] );
        m_edgeBox.draw();
    }
    m_edgeBox.release();

    /*

    m_shaderProgram->setUniformValue("hMatrix", m_hMatrix );  
    m_edgeBox.draw();

    QMatrix4x4 rot; rot.setToIdentity(); rot.rotate(90, QVector3D(1,0,0));
    m_shaderProgram->setUniformValue("hMatrix", m_hMatrix * rot);  
    m_edgeBox.draw();

    rot.setToIdentity(); rot.rotate(90, QVector3D(0,0,-1));
    m_shaderProgram->setUniformValue("hMatrix", m_hMatrix * rot);  
    m_edgeBox.draw();


    QMatrix4x4 mat,dmat;
    dmat = m_grid->dirMatrices[2];
    mat = dmat;
    for(int i=0;i<10;i++)
    {
        m_shaderProgram->setUniformValue("hMatrix", m_hMatrix * mat);
        mat = mat * dmat;
        m_edgeBox.draw();
    }

    dmat = m_grid->dirMatrices[2];
    mat.setToIdentity();
    for(int i=0;i<10;i++)
    {
        m_shaderProgram->setUniformValue("hMatrix", m_hMatrix * m_grid->dirMatrices[0] * mat);
        m_edgeBox.draw();
        m_shaderProgram->setUniformValue("hMatrix", m_hMatrix * m_grid->dirMatrices[1] * mat);
        m_edgeBox.draw();
        m_shaderProgram->setUniformValue("hMatrix", m_hMatrix * m_grid->dirMatrices[4] * mat);
        m_edgeBox.draw();
        m_shaderProgram->setUniformValue("hMatrix", m_hMatrix * m_grid->dirMatrices[5] * mat);
        m_edgeBox.draw();
        QMatrix4x4 ugh;
        ugh = m_grid->dirMatrices[0] * m_grid->dirMatrices[4];
        m_shaderProgram->setUniformValue("hMatrix", m_hMatrix * ugh * mat);
        m_edgeBox.draw();
        ugh = m_grid->dirMatrices[4] * m_grid->dirMatrices[0];
        m_shaderProgram->setUniformValue("hMatrix", m_hMatrix * ugh * mat);
        m_edgeBox.draw();
        
        mat = mat * dmat;
        
    }

    

    m_edgeBox.release();    
    */


    glDisableClientState(GL_VERTEX_ARRAY);
    glDisableClientState(GL_NORMAL_ARRAY);
    glDisable(GL_CULL_FACE);
    m_shaderProgram->release();


    // test();



}


void H3GridPage::drawVertices()
{
    m_vertexCube.bind();    
    m_shaderProgram->setUniformValue("hMatrix", m_hMatrix );  

    m_vertexCube.draw();
    for(int i=0;i<6;i++)
    {
        QMatrix4x4 dir = m_grid->dirMatrices[i];
        QMatrix4x4 mat = m_hMatrix * dir;
        for(int j = 0; j<10; j++) 
        {
            m_shaderProgram->setUniformValue("hMatrix", mat );  
            m_vertexCube.draw();
            mat = mat * dir;
        }
    }

    for(int i=0; i<4; i++)
    {
        for(int j=4; j<6; j++)
        {
            QMatrix4x4 mat = m_hMatrix * m_grid->dirMatrices[i] * m_grid->dirMatrices[j];
            QMatrix4x4 dmat = m_grid->dirMatrices[j];
            for(int k = 0; k<10; k++) 
            {
                m_shaderProgram->setUniformValue("hMatrix", mat );  
                m_vertexCube.draw();
                mat = mat * dmat;
            }
        }
    }


    m_vertexCube.draw();
    m_vertexCube.release();

}

void H3GridPage::test()
{
    

    glEnableClientState(GL_NORMAL_ARRAY);
    glEnableClientState(GL_VERTEX_ARRAY);
    glEnable(GL_CULL_FACE);
    
    m_vertexCube.release();
    
    
    glDisableClientState(GL_VERTEX_ARRAY);
    glDisableClientState(GL_NORMAL_ARRAY);
    glDisable(GL_CULL_FACE);

    return;


    QVector<GLushort> indices;
    QVector<GLfloat> buffer;
    int vCount;
    int m = 10, n = 10;
    for(int i=0; i<n; i++)
    {
        double t = (double)i/(double)(n-1);
        for(int j=0;j<m;j++)
        {
            double s = (double)j/(double)(m-1);
            buffer << (2*s-1) * 0.1 << (2*t-1) * 0.1 << 0;
            buffer << 0 << 0 << -1;
            if(i+1<n && j+1<m) {
                indices 
                    << i*m+j << i*m+j+1 << (i+1)*m+j+1
                    << i*m+j << (i+1)*m+j+1 << (i+1)*m+j;
            }
        }
    }
    glEnableClientState(GL_NORMAL_ARRAY);
    glEnableClientState(GL_VERTEX_ARRAY);
    glEnable(GL_CULL_FACE);
    m_sphere.bind();
    

    //glNormalPointer(GL_FLOAT, sizeof(GL_FLOAT)*6, &buffer[0]+3);
    //glVertexPointer(3, GL_FLOAT, sizeof(GL_FLOAT)*6, &buffer[0]);
   
    
    m_shaderProgram->bind();
    
    double eps = 0.2;
    QMatrix4x4 matx = H3::KModel::translation(QVector3D(0,0,0), QVector3D(eps,0,0));
    QMatrix4x4 maty = H3::KModel::translation(QVector3D(0,0,0), QVector3D(0,eps,0));
    QMatrix4x4 matz = H3::KModel::translation(QVector3D(0,0,0), QVector3D(0,0,eps));

    QMatrix4x4 mati;
    for(int i=0; i<30;i++)
    {
        QMatrix4x4 matj = mati;
        for(int j=0; j<30;j++)
        {
            QMatrix4x4 matk = matj;
            for(int k=0; k<30;k++)
            {
                m_shaderProgram->setUniformValue("hMatrix", m_hMatrix * matk);
                m_sphere.draw();
                matk = matz*matk;
            }
            matj = maty*matj;
        }
        mati = matx * mati;
    }

    

    


    m_sphere.release();
    m_shaderProgram->release();
    
    glDisableClientState(GL_VERTEX_ARRAY);
    glDisableClientState(GL_NORMAL_ARRAY);
    glDisable(GL_CULL_FACE);

}

void H3GridPage::drawDodWf(const QMatrix4x4 &hMatrix)
{
    int m = 50;
    for(int i=0;i<m_grid->m_edges.count();i++)
    {
        const QPair<int,int> &edge = m_grid->m_edges[i];
        glBegin(GL_LINE_STRIP);
        QVector4D p0 = m_grid->m_pts[edge.first], p1 = m_grid->m_pts[edge.second];
        for(int i=0; i<m; i++) {
            double t = (double)i/(double)(m-1);
            glVertex(H3::KModel::toBall(hMatrix*(p0*(1-t)+p1*t)));
        }
        glEnd();
    }
}

void H3GridPage::drawBoxWf(const QMatrix4x4 &hMatrix, double dx, double dy, double dz)
{
    const int m = 10;
    QVector3D pts[8] = {
        QVector3D(-dx,-dy,-dz),
        QVector3D( dx,-dy,-dz),
        QVector3D(-dx, dy,-dz),
        QVector3D( dx, dy,-dz),
        QVector3D(-dx,-dy, dz),
        QVector3D( dx,-dy, dz),
        QVector3D(-dx, dy, dz),
        QVector3D( dx, dy, dz)
    };
    const int edges[12][2] = {
        {0,1},{1,3},{3,2},{2,0},
        {4,5},{5,7},{7,6},{6,4},
        {0,4},{1,5},{2,6},{3,7}
    };
    for(int i=0;i<12;i++)
    {
        QVector3D p0 = pts[edges[i][0]], p1 = pts[edges[i][1]];
        glBegin(GL_LINE_STRIP);
        for(int j=0;j<m;j++)
        {
            double t = (double)j/(double)(m-1);
            glVertex(H3::KModel::toBall(QVector4D(hMatrix*(p0*(1-t)+p1*t), 1.0)));
        }
        glEnd();
    }
}

void H3GridPage::drawEdgeWf(const QMatrix4x4 &hMatrix, double d)
{
    double edgeLength = m_grid->edgeLength;
    
    int n = 10;
    QVector4D base[4] = {
        QVector4D(-d,0,-d,1.0),
        QVector4D( d,0,-d,1.0),
        QVector4D( d,0, d,1.0),
        QVector4D(-d,0, d,1.0)
    };

    QVector<QVector4D> pts;
    for(int i=0; i<=2*n; i++) 
    {
        double hz = 0.5*edgeLength*(1+(double)(i-n)/(double)n);
        QMatrix4x4 tr = H3::KModel::translation(
            QVector3D(0,0,0), 
            QVector3D(0,H3::KModel::getRadius(hz), 0));
        for(int j=0; j<4; j++)
        {
            pts.append(tr.map(base[j]));
        }
    }

    QVector<QVector3D> pts_e;
    for(int i=0;i<pts.count();i++) pts_e.append(H3::KModel::toBall(hMatrix.map(pts[i])));

    for(int j=0; j<4; j++)
    {
        glBegin(GL_LINE_STRIP);
        for(int i=0; i<=2*n; i++)
        {
            glVertex(pts_e[i*4+j]);
        }
        glEnd();
    }
    glBegin(GL_LINE_STRIP);
    for(int j=0; j<=4; j++) glVertex(pts_e[j%4]);
    glEnd();
    glBegin(GL_LINE_STRIP);
    for(int j=0; j<=4; j++) glVertex(pts_e[2*n*4+(j%4)]);
    glEnd();

}


void H3GridPage::drawBoundary()
{
    glDisable(GL_LIGHTING);
    glColor3d(0,0.2,0.6);
    int m = 50;
    double r = 5.0;
    QVector<QPair<double, double> > cssn;
    for(int i=0;i<m;i++) {
        double phi = M_PI * 2 * i / m;
        cssn.append(qMakePair(cos(phi)*r, sin(phi)*r));
    }
    glBegin(GL_LINES);
    for(int i=0;i<m;i++) 
    {
        int i1 = (i+1)%m;
        double a = cssn[i].first, b = cssn[i].second, c = cssn[i1].first, d = cssn[i1].second;
        glVertex3d(a,b,0); glVertex3d(c,d,0);
        glVertex3d(a,0,b); glVertex3d(c,0,d);
        glVertex3d(0,a,b); glVertex3d(0,c,d);
    }
    glEnd();

    glBegin(GL_LINES);
        glColor3d(1,0,0);
        glVertex3d(0,0,0);
        glVertex3d(r,0,0);
        glColor3d(0,1,0);
        glVertex3d(0,0,0);
        glVertex3d(0,r,0);
        glColor3d(0,0,1);
        glVertex3d(0,0,0);
        glVertex3d(0,0,r);

    glEnd();

    glEnable(GL_LIGHTING);
}


void H3GridPage::mousePressEvent(QMouseEvent *e)
{
  m_lastPos = e->pos();
  m_rotating = e->button() == Qt::RightButton;
}

void H3GridPage::mouseReleaseEvent(QMouseEvent *e)
{

}

void H3GridPage::mouseMoveEvent(QMouseEvent *e)
{
  QPoint delta = m_lastPos - e->pos();
  m_lastPos = e->pos();
  if(!m_rotating)
  {
      m_hOffset += QVector3D(delta.x()*0.01, delta.y()*0.01, 0.0);
      m_hMatrix = H3::KModel::translation(m_hOffset, QVector3D(0,0,0));

  }
  else
  {
    m_phi -= 0.25*delta.x();
    m_theta -= 0.25*delta.y();
  }
  updateGL();
}

void H3GridPage::keyPressEvent(QKeyEvent *e)
{
    e->ignore();
}

void H3GridPage::wheelEvent(QWheelEvent*e)
{
  m_cameraDistance = clamp(m_cameraDistance - e->delta() * 0.01, 1,50);
  updateGL();
}

void H3GridPage::showEvent(QShowEvent*)
{
  setFocus();
}

void H3GridPage::hideEvent(QHideEvent*)
{
}

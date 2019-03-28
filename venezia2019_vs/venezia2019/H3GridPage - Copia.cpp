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


#include "Viewer.h"
// #include <QGLWidget>

H3GridPage::H3GridPage()
: m_theta(0)
, m_phi(0)
, m_cameraDistance(10)
, m_rotating(true)
, m_shaderProgram(0)
, m_grid(new H3Grid534())
, m_level(0)

{
    m_grid2 = new H3Grid(*m_grid);
    buildGrid();

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
 
  m_sphere.makeSphere(0.3,10,10);
  
  m_vertexCube.makeCube(0.1,8);

  makeEdgeBox();
  
  build();
  // m_vertexCube.makeBox(0.1,0.2,0.3,5,10,15);
  m_clock.start();
}

void H3GridPage::build()
{
    /*
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
    */
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
    // drawBackground();

    glPushMatrix();
    glTranslated(0,0,-m_cameraDistance);
    glRotated(m_theta,1,0,0);
    glRotated(m_phi,0,1,0);


    GLfloat specular[] =  { 0.7f, 0.7f, 0.7f, 1.0f};
    glMaterialfv(GL_FRONT_AND_BACK, GL_SPECULAR, specular);
    glMaterialf(GL_FRONT_AND_BACK, GL_SHININESS, 90.0);
    // glMaterialfv(GL_FRONT_AND_BACK, GL_AMBIENT_AND_DIFFUSE, colors[0]);

    draw3();


    glPopMatrix();
}

void H3GridPage::draw(const QMatrix4x4 &mat, const Mesh &mesh)
{
    m_shaderProgram->setUniformValue("hMatrix", m_hMatrix * mat );
    mesh.draw();
}


void H3GridPage::draw(const QMatrix4x4 &globalMatrix, const GridMatrices &matrices)
{
    if(!matrices.m_vertexMatrices.empty())
    {
        m_vertexCube.bind();
        foreach(QMatrix4x4 mat, matrices.m_vertexMatrices) {
            draw(globalMatrix * mat, m_vertexCube);
        }
        m_vertexCube.release();
    }
    if(!matrices.m_edgeMatrices.empty())
    {
        m_edgeBox.bind();    
        foreach(QMatrix4x4 mat, matrices.m_edgeMatrices) {
            draw(globalMatrix * mat, m_edgeBox);
        }
        m_edgeBox.release();
    }
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
        
    drawAxes();
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
    
    /*
    // vertices
    m_vertexCube.bind();    
    for(int i=0;i<m_vertexMatrices.count();i++) {
        m_shaderProgram->setUniformValue("hMatrix", m_hMatrix * m_vertexMatrices[i] );
        m_vertexCube.draw();
    }
    m_vertexCube.release();
    */

    /*

    // edges
    m_edgeBox.bind();    
    for(int i=0;i<m_edgeMatrices.count();i++) {
        m_shaderProgram->setUniformValue("hMatrix", m_hMatrix * m_edgeMatrices[i] );
        m_edgeBox.draw();
    }
    m_edgeBox.release();


    // vertices
    m_vertexCube.bind();    
    for(int i=0;i<20;i++) {
        draw(m_vertexCube,  hMatrix * m_grid->getCellVertexMatrix(i));
    }
    m_vertexCube.release();
    */



    // GridMatrices matrices;
    // m_grid->addDodVertices(matrices);
    // m_grid->addEdgeAndVertex(matrices, mat);

    draw(identity, m_gridMatrices);






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


void H3GridPage::wheelEvent(QWheelEvent*e)
{
  m_cameraDistance = clamp(m_cameraDistance - e->delta() * 0.01, 1,50);
  updateGL();
}


void H3GridPage::keyPressEvent(QKeyEvent *e)
{
    /*
    if(e->key() == Qt::Key_Plus) { 
        m_level++; 
        m_gridMatrices.clear();
        QMatrix4x4 identity; identity.setToIdentity();
        m_grid->flower(m_gridMatrices, identity, m_level);
        qDebug() << m_gridMatrices.m_edgeMatrices.count() << " edges" << m_gridMatrices.m_vertexMatrices.count() << " vertices";
        updateGL(); 
    }
    else if(e->key() == Qt::Key_Minus) { 
        if(m_level>0) m_level--; 
        m_gridMatrices.clear();
        QMatrix4x4 identity; identity.setToIdentity();
        m_grid->flower(m_gridMatrices, identity, m_level);
        qDebug() << m_gridMatrices.m_edgeMatrices.count() << " edges" << m_gridMatrices.m_vertexMatrices.count() << " vertices";
        updateGL(); 
    }
    */
    if(e->key() == Qt::Key_Plus) { 
        test3();
    }
    else
      e->ignore();
}


void H3GridPage::test3()
{

}

void H3GridPage::buildGrid()
{
    QTime clock;
    clock.start();
    m_grid2->createFirstVertex();

    // 7
    for(int step=0; step<5; step++)
    {
        QList<QPair<int, int> > todo;
        for(int i=0;i<m_grid2->m_vertices.count();i++)
        {
            for(int j=0;j<6;j++)
            {
                if(m_grid2->m_vertices[i].links[j]== -1) todo.append(qMakePair(i,j));
            }
        }
        qDebug() << "todo size = " << todo.count();
        for(int i=0;i<todo.count();i++)
        {
            int vIndex = todo[i].first;
            int dir = todo[i].second;
            m_grid2->closeIfNeeded(vIndex, dir);
            if(m_grid2->m_vertices[vIndex].links[dir]>=0) continue;
            m_grid2->addVertex(vIndex, dir);
        }
        qDebug() << "Step finished. vertex count = " << m_grid2->m_vertices.count();
    }

    // chiudo tutto
    for(int i=0;i<m_grid2->m_vertices.count();i++)
    {
        for(int j=0;j<6;j++)
        {
            m_grid2->closeIfNeeded(i, j);
        }
    }

    /*

    for(int i=0;i<6;i++)
        m_grid2->addVertex(0,i);

    for(int i=1;i<=6;i++)
        for(int j=1;j<=4;j++)
            m_grid2->addVertex(i,j);
            */


//     m_grid2->foo(7);
//    for(int i=7; i<7+24; i++)
//         m_grid2->foo(i);

    qDebug() << "GRID BUILD: "<< clock.elapsed();
    /*

    for(int i=0;i<m_grid2->m_vertices.count();i++)
    {
        for(int j=0;j<24;j++)
        {
            QList<int> face;
            int m = m_grid2->getFace(face, i, j);
            if(m>1)
            {
                qDebug() << m << face;
            }
        }
    }
    */
    /*
    // m_grid2->addVertex(m_grid2->m_edges[1]);
    */
}


//------------------------
//
// draw
//
//------------------------


void H3GridPage::draw3()
{
    // drawAxes();

    GLdouble viewArr[16];
    glGetDoublev(GL_MODELVIEW_MATRIX, viewArr);
    QMatrix4x4 view(viewArr);


    m_shaderProgram->bind();
    setViewUniforms(m_shaderProgram);    
    glEnableClientState(GL_NORMAL_ARRAY);
    glEnableClientState(GL_VERTEX_ARRAY);
    glEnable(GL_CULL_FACE);
    
    QMatrix4x4 globalMatrix; globalMatrix.setToIdentity();


    
    m_vertexCube.bind();
    for(int i=0;i<m_grid2->m_vertices.count();i++) {
        const QMatrix4x4 &mat = m_grid2->m_vertices.at(i).matrix;
        
        draw(globalMatrix * mat, m_vertexCube);
    }
    m_vertexCube.release();
        
    m_edgeBox.bind();    
    for(int i=0;i<m_grid2->m_edgeMatrices.count();i++) {
        const QMatrix4x4 &mat = m_grid2->m_edgeMatrices.at(i);
        draw(globalMatrix * mat, m_edgeBox);
    }
    m_edgeBox.release();
    

    glDisableClientState(GL_VERTEX_ARRAY);
    glDisableClientState(GL_NORMAL_ARRAY);
    glDisable(GL_CULL_FACE);
    m_shaderProgram->release();


    // setColor(1,0.5,0.0);

    // drawChannels();

    
    /*
    for(int i=1;i<m_grid2->m_edges.count();i++) {
        const H3Grid::Edge &edge = m_grid2->m_edges.at(i);
        const QMatrix4x4 &mat = globalMatrix* edge.matrix;

        double d = 0.1, y = 0.3;
        const QVector4D edgeFacePts[4] = {
            QVector4D(d,y,0,1),
            QVector4D(0,y,d,1),
            QVector4D(-d,y,0,1),
            QVector4D(0,y,-d,1)
        };
        for(int j=0; j<4; j++) {
            QVector3D p = H3::KModel::toBall(mat.map(edgeFacePts[j]));
            v()->drawText(p, QString::number(edge.sides[j]), 0.125, Qt::black);
        }

    }
    */
    

    /*

    glEnable(GL_TEXTURE_2D);
    glBindTexture(GL_TEXTURE_2D, 1);
    glBegin(GL_QUADS);
    glTexCoord2f(0.0f, 1.0f); glVertex3d(p.x()-1,p.y()-1,p.z());
    glTexCoord2f(1.0f, 1.0f); glVertex3d(p.x()+1,p.y()-1,p.z());
    glTexCoord2f(1.0f, 0.0f); glVertex3d(p.x()+1,p.y()+1,p.z());
    glTexCoord2f(0.0f, 0.0f); glVertex3d(p.x()-1,p.y()+1,p.z());
    glEnd();
    glDisable(GL_TEXTURE_2D);

    // v()->renderText(p.x(),p.y(),p.z(),"ecco");
    */
}

void H3GridPage::drawChannels()
{
    QMatrix4x4 globalMatrix = m_hMatrix; //  globalMatrix.setToIdentity();

    const double t = 0.2, s = 0.1;
    const double pts[][3] = {
        {s,t,0}, {0,t,s}, {-s,t,0}, {0,t,-s},
        {t,s,0}, {t,0,-s}, {t,-s,0}, {t,0,s},
        {0,s,t}, {s,0,t}, {0,-s,t}, {-s,0,t},
        {-t,s,0}, {-t,0,s}, {-t,-s,0}, {-t,0,-s}, 
        {0,s,-t},{-s,0,-t},{0,-s,-t},{s,0,-t},
        {-s,-t,0},{0,-t,s},{s,-t,0},{0,-t,-s}
    };
    double sc = 0.9;

    glDisable(GL_LIGHTING);

    glBegin(GL_LINES);

    for(int i=0;i<m_grid2->m_vertices.count();i++)
    {
        const H3Grid::Vertex &vertex0 = m_grid2->m_vertices[i];
        const QMatrix4x4 &mat0 = globalMatrix * vertex0.matrix;


        for(int c0=0; c0<24;c0++) {
            int c1 = vertex0.channels[c0];
            // links consistente con channels
            assert((c1>=0) == (vertex0.links[c0/4]>=0));
            if(c1<0) continue;

            const H3Grid::Vertex &vertex1 = m_grid2->m_vertices[vertex0.links[c0/4]];
            const QMatrix4x4 &mat1 = globalMatrix *  vertex1.matrix;

            QVector3D p0 = H3::KModel::toBall(mat0.map( QVector4D(pts[c0][0],pts[c0][1],pts[c0][2],1) ));
            QVector3D p1 = H3::KModel::toBall(mat1.map( QVector4D(sc*pts[c1][0],sc*pts[c1][1],sc*pts[c1][2],1) ));

            glColor3d(1,0,1);
            glVertex3d(p0.x(),p0.y(),p0.z());
            glColor3d(0,1,1);
            glVertex3d(p1.x(),p1.y(),p1.z());
        }

        for(int c0=0; c0<24;c0++) {
            int c1 = H3Grid::innerLink(c0);
            QVector3D p0 = H3::KModel::toBall(mat0.map( QVector4D(pts[c0][0],pts[c0][1],pts[c0][2],1) ));
            QVector3D p1 = H3::KModel::toBall(mat0.map( QVector4D(sc*pts[c1][0],sc*pts[c1][1],sc*pts[c1][2],1) ));

            glColor3d(0.5,0.5,0.5);
            glVertex3d(p0.x(),p0.y(),p0.z());
            glColor3d(1,1,1);
            glVertex3d(p1.x(),p1.y(),p1.z());
        }
    }
    glEnd();
    glEnable(GL_LIGHTING);
}

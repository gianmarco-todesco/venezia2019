#include "Fig12Page.h"



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

Fig12Page::Fig12Page()
: m_theta(10)
, m_phi(20)
, m_cameraDistance(5)
, m_rotating(true)
, m_shaderProgram(0)
, m_grid(new H3Grid534())
, m_level(0)

{
    m_grid2 = new H3Grid(*m_grid);
    buildGrid();

    m_hMatrix.setToIdentity();
}

Fig12Page::~Fig12Page()
{
    delete m_shaderProgram; m_shaderProgram=0;
    delete m_grid; m_grid = 0;
}

void Fig12Page::initializeGL()
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
 
  m_sphere.makeSphere(0.3,10,10);
  
  m_vertexCube.makeCube(0.1,8);

  makeEdgeBox(m_edgeBox, 10);
  makeEdgeBox(m_edgeBoxLow, 1);
  
  m_clock.start();
}


void Fig12Page::resizeGL(int width, int height)
{
  double aspect = (float)width/height;
  glViewport(0,0,width,height);
  glMatrixMode(GL_PROJECTION);
  glLoadIdentity();
  gluPerspective(50, aspect, 1.0, 60.0);
  glMatrixMode(GL_MODELVIEW);
  glLoadIdentity();
}

void Fig12Page::makeEdgeBox(Mesh &mesh, int n)
{
    double d = 0.05;
    double edgeLength = m_grid->edgeLength;    
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


void Fig12Page::paintGL()
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

void Fig12Page::draw(const QMatrix4x4 &mat, const Mesh &mesh)
{
    m_shaderProgram->setUniformValue("hMatrix", m_hMatrix * mat );
    mesh.draw();
}


void Fig12Page::drawDodWf(const QMatrix4x4 &hMatrix)
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

void Fig12Page::drawBoxWf(const QMatrix4x4 &hMatrix, double dx, double dy, double dz)
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

void Fig12Page::drawEdgeWf(const QMatrix4x4 &hMatrix, double d)
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



void Fig12Page::mousePressEvent(QMouseEvent *e)
{
  m_lastPos = e->pos();
  m_rotating = e->button() == Qt::RightButton;
}

void Fig12Page::mouseReleaseEvent(QMouseEvent *e)
{

}

void Fig12Page::mouseMoveEvent(QMouseEvent *e)
{
  QPoint delta = m_lastPos - e->pos();
  m_lastPos = e->pos();
  if(!m_rotating)
  {
      // m_hOffset += QVector3D(delta.x()*0.01, delta.y()*0.01, 0.0);
      // m_hMatrix = H3::KModel::translation(m_hOffset, QVector3D(0,0,0));

      m_hOffset += QVector3D(0,0,delta.y()*0.01);
      m_hMatrix = H3::KModel::translation(m_hOffset, QVector3D(0,0,0));
  }
  else
  {
    m_phi -= 0.25*delta.x();
    m_theta -= 0.25*delta.y();
  }
  updateGL();
}


void Fig12Page::wheelEvent(QWheelEvent*e)
{
  m_cameraDistance = clamp(m_cameraDistance - e->delta() * 0.01, 1,50);
  updateGL();
}


void Fig12Page::keyPressEvent(QKeyEvent *e)
{
    if(e->key() == Qt::Key_Z) { 
        double r = -H3::KModel::getRadius(m_grid->edgeLength);
        if(e->modifiers() & Qt::ShiftModifier) r=-r;
        m_hMatrix = m_hMatrix * H3::KModel::translation(QVector3D(0,0,0), QVector3D(0,0,r));
    }
    else
      e->ignore();
}

void Fig12Page::buildGrid()
{
    QTime clock;
    clock.start();
    m_grid2->createFirstVertex();

    // 7
    for(int step=0; step<7; step++)
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
    qDebug() << "GRID BUILD: "<< clock.elapsed();
    
}


//------------------------
//
// draw
//
//------------------------

inline bool check(const QMatrix4x4 &mpv, const QMatrix4x4 &hMatrix, const QMatrix4x4 &mat)
{
    QVector4D p4 = (hMatrix * mat).map(QVector4D(0,0,0,1));
    QVector3D p = H3::KModel::toBall(p4, 10);
    QVector4D q4 = mpv.map(QVector4D(p, 1.0));
    QVector3D q = q4.toVector3DAffine();

    /*
    QVector3D p1 = H3::KModel::toBall((hMatrix * mat).map(QVector4D(0.1,0,0,1)), 10);
    QVector3D p2 = H3::KModel::toBall((hMatrix * mat).map(QVector4D(-0.1,0,0,1)), 10);
    if((p1-p2).length()<0.1)
    */

    if(p4.toVector3DAffine().length()>0.95)
    {
        return -1.4<=q.x() && q.x()<=1.4 && 
               -1.4<=q.y() && q.y()<=1.4 && 
               q.z() > 0.0;
    }
    else return true;

    return  -4.0<=q.x() && q.x()<=4.0 && 
            -4.0<=q.y() && q.y()<=4.0; //  &&
            //-0.5 <= q.z() && q.z() <= 0.9;
    
}

void Fig12Page::draw3()
{
    // drawAxes();

    qreal viewArr[16], projArr[16];
    glGetDoublev(GL_MODELVIEW_MATRIX, viewArr);
    glGetDoublev(GL_PROJECTION_MATRIX, projArr);
    QMatrix4x4 view(viewArr), proj(projArr);

    QMatrix4x4 projView = proj.transposed() * view.transposed();

    QMatrix4x4 identity; identity.setToIdentity();


    m_shaderProgram->bind();
    // setViewUniforms(m_shaderProgram);    
    glEnableClientState(GL_NORMAL_ARRAY);
    glEnableClientState(GL_VERTEX_ARRAY);
    glEnable(GL_CULL_FACE);
    
    QMatrix4x4 globalMatrix; globalMatrix.setToIdentity();

    

    
    m_vertexCube.bind();

    
    for(int i=0;i<m_grid2->m_vertices.count();i++) {
        const QMatrix4x4 &mat = m_grid2->m_vertices.at(i).matrix;  
        if(check(projView, m_hMatrix, mat))
          draw(mat, m_vertexCube);
    }
         
    
    m_edgeBox.bind();    
    for(int i=0;i<m_grid2->m_edgeMatrices.count();i++) {
        const QMatrix4x4 &mat = m_grid2->m_edgeMatrices.at(i);
        if(check(projView, m_hMatrix, mat))
            draw(mat, m_edgeBox);
    }
    m_edgeBox.release();
    

    glDisableClientState(GL_VERTEX_ARRAY);
    glDisableClientState(GL_NORMAL_ARRAY);
    glDisable(GL_CULL_FACE);
    m_shaderProgram->release();


    // setColor(1,0.5,0.0);

    // drawChannels();

    static double zz = 0.0;

    zz += .01;
    if(zz>m_grid->edgeLength) zz -= m_grid->edgeLength;

    double r = H3::KModel::getRadius(fabs(zz));

    
 // ggg   m_hMatrix = H3::KModel::translation(QVector3D(0,0,0), QVector3D(0,0,zz > 0 ? r : -r));

}

void Fig12Page::drawChannels()
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


void Fig12Page::savePictures()
{
    Fig12Page page;
    page.m_cameraDistance = m_cameraDistance;
    page.m_theta = m_theta;
    page.m_phi = m_phi;
    page.savePicture("fig12.png");
}

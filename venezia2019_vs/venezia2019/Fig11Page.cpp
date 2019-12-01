#include "Fig11Page.h"

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


Fig11Page::Fig11Page()
    : m_theta(23.5)
    , m_phi(-25.75)
    , m_cameraDistance(26.6)
    , m_gridData(new H3Grid534())
    , m_grid(new H3Grid(*m_gridData))
    , m_rotating(false)
{
    m_hMatrix.setToIdentity();
    buildGrid();
}
    
Fig11Page::~Fig11Page()
{
    delete m_grid; m_grid = 0;
    delete m_gridData; m_gridData = 0;
    delete m_shaderProgram; m_shaderProgram=0;
}

void Fig11Page::buildGrid()
{
    QTime clock;
    clock.start();
    m_grid->createFirstVertex();

    const int n = 9;
    for(int step=0; step<n; step++)
    {
        QList<QPair<int, int> > todo;
        for(int i=0;i<m_grid->m_vertices.count();i++)
        {
            for(int j=0;j<6;j++)
            {
                if(m_grid->m_vertices[i].links[j]== -1) 
                    todo.append(qMakePair(i,j));
            }
        }
        qDebug() << "todo size = " << todo.count();
        for(int i=0;i<todo.count();i++)
        {
            int vIndex = todo[i].first;
            int dir = todo[i].second;
            m_grid->closeIfNeeded(vIndex, dir);
            if(m_grid->m_vertices[vIndex].links[dir]>=0) continue;
            m_grid->addVertex(vIndex, dir);
        }
        qDebug() << "Step finished. vertex count = " << m_grid->m_vertices.count();
    }

    // chiudo tutto
    for(int i=0;i<m_grid->m_vertices.count();i++)
    {
        for(int j=0;j<6;j++)
        {
            m_grid->closeIfNeeded(i, j);
        }
    }
    qDebug() << "GRID BUILD: "<< clock.elapsed();
 
    QVector3D p = m_gridData->getCornerMatrix().map(QVector3D(0,0,0));
    m_hMatrix = H3::KModel::translation(p, QVector3D(0,0,0));
}

void Fig11Page::makeEdgeBox(Mesh &mesh, int n)
{
    double d = 0.03;
    double edgeLength = m_gridData->edgeLength;    
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

void Fig11Page::draw(const QMatrix4x4 &mat, const Mesh &mesh)
{
    m_shaderProgram->setUniformValue("hMatrix", m_hMatrix * mat );
    mesh.draw();
}
  
void Fig11Page::initializeGL() 
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

    makeEdgeBox(m_edgeBox, 10);
    
}

void Fig11Page::resizeGL(int width, int height) 
{
    double aspect = (float)width/height;
    glViewport(0,0,width,height);
    glMatrixMode(GL_PROJECTION);
    glLoadIdentity();
    gluPerspective(45, aspect, 1.0, 70.0);
    glMatrixMode(GL_MODELVIEW);
    glLoadIdentity();
}

void Fig11Page::paintGL() 
{        
    glClearColor(1,1,1,1);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    
    glPushMatrix();
    glTranslated(0,0,-m_cameraDistance);
    glRotated(m_theta,1,0,0);
    glRotated(m_phi,0,1,0);


    qreal viewArr[16], projArr[16];
    glGetDoublev(GL_MODELVIEW_MATRIX, viewArr);
    glGetDoublev(GL_PROJECTION_MATRIX, projArr);
    QMatrix4x4 view(viewArr), proj(projArr);
    QMatrix4x4 projView = proj.transposed() * view.transposed();
    QMatrix4x4 identity; identity.setToIdentity();

    m_shaderProgram->bind();
    glEnableClientState(GL_NORMAL_ARRAY);
    glEnableClientState(GL_VERTEX_ARRAY);
    glEnable(GL_CULL_FACE);
    
    QMatrix4x4 globalMatrix; globalMatrix.setToIdentity();

    /*
    m_vertexCube.bind();

    
    for(int i=0;i<m_grid2->m_vertices.count();i++) {
        const QMatrix4x4 &mat = m_grid2->m_vertices.at(i).matrix;  
        if(check(projView, m_hMatrix, mat))
          draw(mat, m_vertexCube);
    }
    */

    
    m_edgeBox.bind();    
    for(int i=0;i<m_grid->m_edgeMatrices.count();i++) {
        const QMatrix4x4 &mat = m_grid->m_edgeMatrices.at(i);
        draw(mat, m_edgeBox);
    }
    m_edgeBox.release();
    

    glDisableClientState(GL_VERTEX_ARRAY);
    glDisableClientState(GL_NORMAL_ARRAY);
    glDisable(GL_CULL_FACE);
    m_shaderProgram->release();
 
    glPopMatrix();
}




void Fig11Page::mousePressEvent(QMouseEvent *e)
{
    m_lastPos = e->pos();
    m_rotating = e->button() == Qt::RightButton;
}

void Fig11Page::mouseReleaseEvent(QMouseEvent *e)
{

}

void Fig11Page::mouseMoveEvent(QMouseEvent *e)
{
  QPoint delta = m_lastPos - e->pos();
  m_lastPos = e->pos();
  if(!m_rotating)
  {

      // m_hOffset += QVector3D(0,0,delta.y()*0.01);
      // m_hMatrix = H3::KModel::translation(m_hOffset, QVector3D(0,0,0));
  }
  else
  {
    m_phi -= 0.25*delta.x();
    m_theta -= 0.25*delta.y();
  }
  updateGL();
}


void Fig11Page::wheelEvent(QWheelEvent*e)
{
  m_cameraDistance = clamp(m_cameraDistance - e->delta() * 0.01, 1,50);
  updateGL();
}


void Fig11Page::keyPressEvent(QKeyEvent *e)
{
    if(e->key() == Qt::Key_Z) { 
        double r = -H3::KModel::getRadius(m_gridData->edgeLength);
        if(e->modifiers() & Qt::ShiftModifier) r=-r;
        m_hMatrix = m_hMatrix * H3::KModel::translation(QVector3D(0,0,0), QVector3D(0,0,r));
    }
    else
      e->ignore();
}


void Fig11Page::savePictures()
{
    Fig11Page page;
    page.m_cameraDistance = m_cameraDistance;
    page.m_theta = m_theta;
    page.m_phi = m_phi;
    page.savePicture("fig11.png");
}
    
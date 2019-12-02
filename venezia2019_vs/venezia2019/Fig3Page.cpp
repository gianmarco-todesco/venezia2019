#include "Fig3Page.h"

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


// fig.3 The "never happening" Denis' polyhedron. Photo courtesy of Maria Dedò and Luisa Colosio.


Fig3Page::Fig3Page()
    : m_theta(23.5)
    , m_phi(-25.75)
    , m_cameraDistance(24.2)
    , m_rotating(false)
{
    m_hMatrix.setToIdentity();
}
    
Fig3Page::~Fig3Page()
{
}

  
void Fig3Page::initializeGL() 
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

    // m_shaderProgram = loadProgram("h3grid");
    
}

void Fig3Page::resizeGL(int width, int height) 
{
    double aspect = (float)width/height;
    glViewport(0,0,width,height);
    glMatrixMode(GL_PROJECTION);
    glLoadIdentity();
    gluPerspective(45, aspect, 1.0, 70.0);
    glMatrixMode(GL_MODELVIEW);
    glLoadIdentity();
}

void Fig3Page::paintGL() 
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

    // m_shaderProgram->bind();
    glEnableClientState(GL_NORMAL_ARRAY);
    glEnableClientState(GL_VERTEX_ARRAY);
    glEnable(GL_CULL_FACE);
    
    QMatrix4x4 globalMatrix; globalMatrix.setToIdentity();

    
    v()->renderText(0,0,0, "Page 3");

    glDisableClientState(GL_VERTEX_ARRAY);
    glDisableClientState(GL_NORMAL_ARRAY);
    glDisable(GL_CULL_FACE);
    // m_shaderProgram->release();
 
    glPopMatrix();
}




void Fig3Page::mousePressEvent(QMouseEvent *e)
{
    m_lastPos = e->pos();
    m_rotating = e->button() == Qt::RightButton;
}

void Fig3Page::mouseReleaseEvent(QMouseEvent *e)
{

}

void Fig3Page::mouseMoveEvent(QMouseEvent *e)
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


void Fig3Page::wheelEvent(QWheelEvent*e)
{
  m_cameraDistance = clamp(m_cameraDistance - e->delta() * 0.01, 1,50);
  updateGL();
}


void Fig3Page::keyPressEvent(QKeyEvent *e)
{
    e->ignore();
}


void Fig3Page::savePictures()
{
    Fig3Page page;
    page.m_cameraDistance = m_cameraDistance;
    page.m_theta = m_theta;
    page.m_phi = m_phi;
    page.savePicture("fig1.png");
}

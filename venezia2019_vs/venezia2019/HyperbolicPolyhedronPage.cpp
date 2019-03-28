#include "HyperbolicPolyhedronPage.h"


#include "HyperbolicPolyhedronPage.h"

#include "Polyhedra.h"
#include "Gutil.h"
#include "Point3.h"

#include "Mesh.h"

#define _USE_MATH_DEFINES
#include <math.h>
#include <assert.h>
#include <QKeyEvent>
#include <QWheelEvent>

#include <vector>

#include <QGLShaderProgram>
#include <qmatrix4x4.h>
#include <qvector.h>
#include <qlist.h>
#include <qmap.h>



HyperbolicPolyhedronPage::HyperbolicPolyhedronPage()
: m_cameraDistance(15)
, m_theta(0)
, m_phi(0)
, m_rotating(true)
{
}

HyperbolicPolyhedronPage::~HyperbolicPolyhedronPage()
{
}

void HyperbolicPolyhedronPage::initializeGL()
{
}

void HyperbolicPolyhedronPage::start()
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

  
    m_clock.start();

}

void HyperbolicPolyhedronPage::resizeGL(int width, int height)
{
    double aspect = (float)width/height;
    glViewport(0,0,width,height);
    glMatrixMode(GL_PROJECTION);
    glLoadIdentity();
    gluPerspective(45, aspect, 1.0, 70.0);
    glMatrixMode(GL_MODELVIEW);
    glLoadIdentity();
}


void HyperbolicPolyhedronPage::paintGL()
{
    glClearColor(0,0,0,1);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    drawBackground();
    
    glPushMatrix();
    glTranslated(0,0,-m_cameraDistance);
    glRotated(m_theta, 1,0,0);
    glRotated(m_phi, 0,1,0);


    GLfloat specular[] =  { 0.7f, 0.7f, 0.7f, 1.0f};
    glMaterialfv(GL_FRONT_AND_BACK, GL_SPECULAR, specular);
    glMaterialf(GL_FRONT_AND_BACK, GL_SHININESS, 90.0);

    draw();
    // drawAxes();

    glEnableClientState(GL_NORMAL_ARRAY);
    glEnableClientState(GL_VERTEX_ARRAY);
    glEnable(GL_CULL_FACE);


    glDisable(GL_CULL_FACE);
    
    glDisableClientState(GL_NORMAL_ARRAY);
    glDisableClientState(GL_VERTEX_ARRAY);

    glPopMatrix();
}
/*
void drawPolygon(int n, double r)
{
    QVector<QVector3D> pts;
    for(int i=0;i<n; i++)
    {
        double phi = 2*M_PI*i/n;
        pts.append(QVector3D(r*cos(phi), r*sin(phi), 0));
    }
    
    for(int i=0;i<n; i++)
    {
        setColor(0.2,0.3,0.9);
        drawSphere(pts[i], 0.03);
        setColor(0.2,0.5,0.9);
        drawCylinder(pts[i], pts[(i+1)%n], 0.02);
    }
}

QVector3D getPos(double r, double unit, int vertexIndex, double xAngle)
{
    QMatrix4x4 mat;
    mat.setToIdentity();
    // 
    mat.rotate(xAngle,1,0,0);
    mat.translate(unit,0,unit);
    mat.rotate(45,0,1,0);
    mat.rotate(vertexIndex * 36, 0,0,1);
    return mat.map(QVector3D(r,0,0));
}
*/


void HyperbolicPolyhedronPage::draw()
{
}



void HyperbolicPolyhedronPage::mousePressEvent(QMouseEvent *e)
{
      m_lastPos = e->pos();
      m_rotating = e->button() == Qt::RightButton;
}

void HyperbolicPolyhedronPage::mouseReleaseEvent(QMouseEvent *e)
{

}


void HyperbolicPolyhedronPage::mouseMoveEvent(QMouseEvent *e)
{
  QPoint delta = m_lastPos - e->pos();
  m_lastPos = e->pos();
  if(!m_rotating)
  {
      
  }
  else
  {
    m_phi -= 0.25*delta.x();
    m_theta -= 0.25*delta.y();

  }
  // updateGL();
}

void HyperbolicPolyhedronPage::keyPressEvent(QKeyEvent *e)
{
    e->ignore();
}

void HyperbolicPolyhedronPage::wheelEvent(QWheelEvent*e)
{
    m_cameraDistance = clamp(m_cameraDistance - e->delta() * 0.01, 1,50);
}

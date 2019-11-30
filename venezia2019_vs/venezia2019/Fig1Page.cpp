#include "Fig1Page.h"

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


// fig.1 A computer rendering inspired by the Escher's lithography "Cubic Space Division (Cubic Space Filling)"

Fig1Page::Fig1Page()
    : m_theta(23.5)
    , m_phi(-25.75)
    , m_cameraDistance(24.2)
    , m_rotating(false)
    , m_gridSmallUnit(0)
    , m_gridBigUnit(1)

{
    m_hMatrix.setToIdentity();
}
    
Fig1Page::~Fig1Page()
{
}


void Fig1Page::buildMesh()
{
    QTime clock;
    clock.start();
    const double r1 = 1.0;
    const double r2 = 0.2;
    const double d = 10;

    int n = 4;
    for(int ix=-n;ix<=n;ix++)
    for(int iy=-n;iy<=n;iy++)
    for(int iz=-n;iz<=n;iz++)
        m_mesh.addBox(QVector3D(ix,iy,iz)*d, r1,r1,r1);

    double q = n*d;
    for(int ix=-n;ix<=n;ix++)
    for(int iy=-n;iy<=n;iy++)
    {
        m_mesh.addBox(QVector3D(ix,iy,0)*d, r2,r2,q);
        m_mesh.addBox(QVector3D(ix,0,iy)*d, r2,q,r2);
        m_mesh.addBox(QVector3D(0,ix,iy)*d, q,r2,r2);
    }
    
    m_mesh.createBuffers();
    m_gridSmallUnit = d;
    m_gridBigUnit = d*2*n;

    qDebug() << "cube grid created: " << clock.elapsed();

    m_offset = QVector3D(d*0.5,d*0.5,d*0.5);
}

  
void Fig1Page::initializeGL() 
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
    buildMesh();
    m_shaderProgram = loadProgram("cubeGrid");
}

void Fig1Page::resizeGL(int width, int height) 
{
    double aspect = (float)width/height;
    glViewport(0,0,width,height);
    glMatrixMode(GL_PROJECTION);
    glLoadIdentity();
    gluPerspective(45, aspect, 1.0, 70.0);
    glMatrixMode(GL_MODELVIEW);
    glLoadIdentity();
}

void Fig1Page::paintGL() 
{        
    glClearColor(1,1,1,1);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    
    glPushMatrix();
    glTranslated(0,0,-m_cameraDistance);
    glRotated(m_theta,1,0,0);
    glRotated(m_phi,0,1,0);


     GLfloat specular[] =  { 0.7f, 0.7f, 0.7f, 1.0f};
    glMaterialfv(GL_FRONT_AND_BACK, GL_SPECULAR, specular);
    glMaterialf(GL_FRONT_AND_BACK, GL_SHININESS, 90.0);

    setColor(1,1,1);
    draw();

    /*

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

    

    glDisableClientState(GL_VERTEX_ARRAY);
    glDisableClientState(GL_NORMAL_ARRAY);
    glDisable(GL_CULL_FACE);
    // m_shaderProgram->release();
    */

    glPopMatrix();
}


void Fig1Page::draw()
{
    glEnableClientState(GL_NORMAL_ARRAY);
    glEnableClientState(GL_VERTEX_ARRAY);
    glEnable(GL_CULL_FACE);


    m_shaderProgram->bind();
    setViewUniforms(m_shaderProgram);

    
    m_mesh.bind();

    QMatrix4x4 mat;
    mat.setToIdentity();
    mat.rotate(-m_phi,0,1,0);
    mat.rotate(-m_theta,1,0,0);

    QVector3D dir = mat.map(QVector3D(0,0,-1));

    int m = 1;
    for(int ix = -m; ix <= m; ix++) 
    for(int iy = -m; iy <= m; iy++) 
    for(int iz = -m; iz <= m; iz++) 
    {
        QVector3D v = m_offset + m_gridBigUnit * QVector3D(ix,iy,iz);
        //if(QVector3D::dotProduct(v,dir)>0) 
        {
            m_shaderProgram->setUniformValue("u_offset", v);
            m_mesh.draw();
        }
    }




    m_mesh.release();

    glDisable(GL_CULL_FACE);
    m_shaderProgram->release();
    
    glDisableClientState(GL_NORMAL_ARRAY);
    glDisableClientState(GL_VERTEX_ARRAY);

  
    // moveOffset(-getCurrentDirection()*0.1);
    /*
    if(m_translating)
    {
        m_translatingSpeed = qMin(1.0, m_translatingSpeed + 0.02);
    }
    else
    {
        m_translatingSpeed = qMax(0.0, m_translatingSpeed - 0.02);
    }
    if(m_translatingSpeed>0.0)
        moveOffset(QVector3D(1,1,1)*0.05*m_translatingSpeed);
        */

}






void Fig1Page::mousePressEvent(QMouseEvent *e)
{
    m_lastPos = e->pos();
    m_rotating = e->button() == Qt::RightButton;
}

void Fig1Page::mouseReleaseEvent(QMouseEvent *e)
{

}

void Fig1Page::mouseMoveEvent(QMouseEvent *e)
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


void Fig1Page::wheelEvent(QWheelEvent*e)
{
  m_cameraDistance = clamp(m_cameraDistance - e->delta() * 0.01, 1,50);
  updateGL();
}


void Fig1Page::keyPressEvent(QKeyEvent *e)
{
    e->ignore();
}


void Fig1Page::savePictures()
{
    Fig1Page page;
    page.m_cameraDistance = m_cameraDistance;
    page.m_theta = m_theta;
    page.m_phi = m_phi;
    page.savePicture("fig1.png");
}

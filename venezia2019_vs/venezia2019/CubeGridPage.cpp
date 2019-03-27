#include "CubeGridPage.h"


#include "Gutil.h"
#include "Mesh.h"
#include "Overlay.h"
#include <assert.h>
#include <QKeyEvent>
#include <QWheelEvent>
#include <QGLShaderProgram>
#include <qmatrix4x4.h>
#include <qvector.h>
#include <qlist.h>

#include <QTime>
#include <qdebug.h>


CubeGridPage::CubeGridPage()
: m_cameraDistance(15)
, m_theta(31.75)
, m_phi(-19.75)
, m_rotating(true)
, m_gridSmallUnit(0)
, m_gridBigUnit(1)
, m_title(0)
, m_escher(0)
{
}

CubeGridPage::~CubeGridPage()
{
    delete m_title;
    delete m_escher;
}

void CubeGridPage::buildMesh()
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

void CubeGridPage::initializeGL()
{
    buildMesh();
    m_shaderProgram = loadProgram("cubeGrid");
    m_title = new OverlayPanel("images/cubic-space-division.jpg");

}

void CubeGridPage::start()
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
}

void CubeGridPage::resizeGL(int width, int height)
{
    double aspect = (float)width/height;
    glViewport(0,0,width,height);
    glMatrixMode(GL_PROJECTION);
    glLoadIdentity();
    gluPerspective(45, aspect, 1.0, 200.0);
    glMatrixMode(GL_MODELVIEW);
    glLoadIdentity();
}


void CubeGridPage::paintGL()
{
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

    setColor(1,1,1);
    draw();
    // drawAxes();

    /*
    setColor(1,0,1);
    drawSphere(2*getCurrentDirection(), 0.2);

    setColor(0,1,1);
    drawSphere(QVector3D(0.2,0,0), 0.1);
    drawSphere(QVector3D(-0.2,0,0), 0.1);
    */

    glEnableClientState(GL_NORMAL_ARRAY);
    glEnableClientState(GL_VERTEX_ARRAY);
    glEnable(GL_CULL_FACE);




    glDisable(GL_CULL_FACE);
    
    glDisableClientState(GL_NORMAL_ARRAY);
    glDisableClientState(GL_VERTEX_ARRAY);

    glPopMatrix();

}

void CubeGridPage::draw()
{
    glEnableClientState(GL_NORMAL_ARRAY);
    glEnableClientState(GL_VERTEX_ARRAY);
    glEnable(GL_CULL_FACE);


    m_shaderProgram->bind();
    setViewUniforms(m_shaderProgram);

    
    m_mesh.bind();

    QVector3D dir = getCurrentDirection();
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
    moveOffset(QVector3D(1,1,1)*0.05);
}



void CubeGridPage::mousePressEvent(QMouseEvent *e)
{
      m_lastPos = e->pos();
      m_rotating = e->button() == Qt::RightButton;
}

void CubeGridPage::mouseReleaseEvent(QMouseEvent *e)
{

}

void CubeGridPage::mouseMoveEvent(QMouseEvent *e)
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
  updateGL();
}

void CubeGridPage::keyPressEvent(QKeyEvent *e)
{
    if(e->key() == Qt::Key_1) m_offset += QVector3D(m_gridSmallUnit,0,0);
    else if(e->key() == Qt::Key_2) m_offset -= QVector3D(m_gridSmallUnit,0,0);
    else if(e->key() == Qt::Key_W) moveOffset(-getCurrentDirection());
    else if(e->key() == Qt::Key_S) moveOffset(getCurrentDirection());
    //else if(e->key() == Qt::Key_3) m_offset += QVector3D(m_gridBigUnit,0,0);
    //else if(e->key() == Qt::Key_4) m_offset -= QVector3D(m_gridBigUnit,0,0);
    else if(e->key() == Qt::Key_Z) {
        getOverlay()->add(m_title);
    }
    e->ignore();
    updateGL();
}

void CubeGridPage::wheelEvent(QWheelEvent*e)
{
    m_cameraDistance = clamp(m_cameraDistance - e->delta() * 0.01, 1,80);
    updateGL();
}

QVector3D CubeGridPage::getCurrentDirection()
{
    QMatrix4x4 mat;
    mat.setToIdentity();
    mat.rotate(-m_phi,0,1,0);
    mat.rotate(-m_theta,1,0,0);

    return mat.map(QVector3D(0,0,-1));
}

double recenter(double x, double u) {
    double t = x/u + 0.5;
    return (t - floor(t) - 0.5)*u;
}


void CubeGridPage::moveOffset(const QVector3D &delta)
{
    const double u = m_gridSmallUnit;
    m_offset += delta;
    m_offset = QVector3D(
        recenter(m_offset.x(), u),
        recenter(m_offset.y(), u),
        recenter(m_offset.z(), u));
}


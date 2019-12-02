#include "CircleLimit3Page.h"
#include "CircleLimit3Viewer.h"
#include "HTransform.h"
#include "Complex.h"
#include "Gutil.h"
#include <QMouseEvent>
#include <QKeyEvent>
#include <QGLPixelBuffer>

using namespace GmLib;

#define GL_MULTISAMPLE 0x809D

CircleLimit3Page::CircleLimit3Page(bool figureMode)
: m_viewer(new CircleLimit3Viewer(figureMode))
, m_timerId(0)
, m_status(0)
, m_transfType(0)
{
}


CircleLimit3Page::~CircleLimit3Page()
{
  delete m_viewer;
}

void CircleLimit3Page::start()
{
    glDisable(GL_DEPTH_TEST);
    glDisable(GL_LIGHTING);
    glTexEnvi(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_DECAL);
    glClearColor(1,1,1,1);
}

void CircleLimit3Page::initializeGL()
{
}

void CircleLimit3Page::resizeGL(int w, int h)
{
    glViewport(0,0,w,h);
    glMatrixMode(GL_PROJECTION);
    glLoadIdentity();
    glOrtho(0,w,0,h,-1,1);
    glMatrixMode(GL_MODELVIEW);
}

void CircleLimit3Page::paintGL()
{
  glClear(GL_COLOR_BUFFER_BIT);

  int w = width(), h = height();

  m_viewer->resize(width(), height(), height()*0.45);

  glColor3d(1,1,1);
  m_viewer->draw();
}

void CircleLimit3Page::mousePressEvent(QMouseEvent *e)
{
  m_lastPos = e->pos();
}

void CircleLimit3Page::mouseMoveEvent(QMouseEvent *e)
{
  QPoint delta = e->pos() - m_lastPos;
  m_lastPos = e->pos();
  Complex c(delta.x(), -delta.y());
  m_viewer->transform(HTransform::translation(0.01*c));
  m_viewer->foo();
  updateGL();
}

void CircleLimit3Page::mouseReleaseEvent(QMouseEvent *)
{
}

void CircleLimit3Page::keyPressEvent(QKeyEvent*e)
{
  if(e->key()=='P' || e->key() == 'O' || e->key() == 'I' )
  {
    if(e->key()=='P') m_transfType = 0;
    else if(e->key()=='O') m_transfType = 1;
    else m_transfType = 2;
  }
  else if(e->key()==Qt::Key_F12)
  {
    savePage();
  }
  else
  {
    if(m_viewer->onKeyDown(e->key()))
      updateGL();
    else
      e->ignore();
  }
}

void CircleLimit3Page::timerEvent(QTimerEvent *)
{
  int d = m_time.restart();
  HTransform transform;
  if(m_transfType==0)
    transform = HTransform::translation((d/40.0)*Complex(0.01,0.009));
  else if(m_transfType ==1)
  {
    Complex c(0.2,0.7);
    transform = HTransform::translation(c) * HTransform::rotation(d*0.0001) * HTransform::translation(-c);
  }
  else
  {
    Complex c0(1,0);
    Complex c1(0,1);
    double k = 1 + d * 0.001;
    Complex den = Complex(1) / (c1-k*c0);
    transform = HTransform(den*(k*c1-c0), den*(1-k) * c0*c1, den*(k-1));
  }

  m_viewer->transform(transform);
  m_viewer->foo();
  updateGL();
}


void CircleLimit3Page::savePage()
{
    /*
  QGLFormat fmt;
  fmt.setDoubleBuffer(false);
  fmt.setAlpha(false);
  QGLPixelBuffer buffer(2048,2048,fmt);
  buffer.makeCurrent();
  m_viewer->loadTextures();

  glEnable(GL_MULTISAMPLE);
  glEnable(GL_BLEND);
  glEnable(GL_LINE_SMOOTH);
  glBlendFunc(GL_SRC_ALPHA,GL_ONE_MINUS_SRC_ALPHA);

  //QPainter painter(&buffer);
  //painter.setRenderHints(QPainter::HighQualityAntialiasing);
  glViewport(0,0,buffer.width(),buffer.height());
  glMatrixMode(GL_PROJECTION);
  glLoadIdentity();
  double r = 140 - 5;
  const QPointF center(200,150);
  glOrtho(center.x()-r,center.x()+r,center.y()-r,center.y()+r,-1,1);
  glMatrixMode(GL_MODELVIEW);
  glClearColor(1,1,1,1);
  // glColorMask(true,true,true,false);
  paintGL();

  glFlush();
  glFinish();
  buffer.doneCurrent();
  makeCurrent();
  buffer.toImage().save("screenshot3.png");
  */
}


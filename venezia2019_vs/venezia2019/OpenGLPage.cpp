#include "OpenGLPage.h"
#include <QPainter>
#include <QDebug>
#include <QGLShaderProgram>
#include <QGLShader>
#include <qfile.h>

OpenGLPage::OpenGLPage()
: QGLWidget(0, getShared())
{
  m_sharedSet.insert(this);
}

OpenGLPage::~OpenGLPage()
{
  m_sharedSet.erase(this);
}

std::set<OpenGLPage*> OpenGLPage::m_sharedSet;

void OpenGLPage::initializeGL()
{
  // glEnable(GL_MULTISAMPLE);
  //glClearColor(0.2,0.3,0.4,1);
  glClearColor(1,1,1,1);
  // QPainter painter(this);
  // painter.setRenderHints(QPainter::HighQualityAntialiasing);
  

}

void OpenGLPage::resizeGL(int width, int height)
{
  glViewport(0,0,width,height);
  glMatrixMode(GL_PROJECTION);
  glLoadIdentity();
  glOrtho(0,width,0,height,-1,1);
  glMatrixMode(GL_MODELVIEW);
}

QGLShaderProgram *OpenGLPage::loadProgram(QString name)
{
    /*
  QFile file(QString("resources/") + name + ".vsh");
  QTextStream in(&file);
  QString text = in.readAll();
  */

  QGLShaderProgram *program = new QGLShaderProgram();
  bool ok = true;
  ok = program->addShaderFromSourceFile(QGLShader::Vertex, "resources/" + name + ".vsh") && ok;
  ok = program->addShaderFromSourceFile(QGLShader::Fragment, "resources/" + name + ".fsh") && ok;
  ok = program->link() && ok;
  if(!ok) 
  {      
      abort();
  }
  return program;
}


void OpenGLPage::drawBackground()
{
  double rx=60, ry = 30, z = -69;
  glDisable(GL_LIGHTING);
  glBegin(GL_QUAD_STRIP);
  glColor3d(.8,.8,.8);
  glVertex3d(-rx,-ry,z);
  glColor3d(.8,.8,.8);
  glVertex3d( rx,-ry,z);
  glColor3d(.7,.9,.9);
  glVertex3d(-rx,ry,z);
  glColor3d(.7,.9,1.0);
  glVertex3d( rx,ry,z);
  glEnd();
  glEnable(GL_LIGHTING);
}


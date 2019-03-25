#include "OpenGLPage.h"
#include <QPainter>
#include <QDebug>
#include <QGLShaderProgram>
#include <QGLShader>
#include <qfile.h>
#define _USE_MATH_DEFINES
#include <math.h>

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

void OpenGLPage::drawAxes(double r)
{
    glDisable(GL_LIGHTING);
    int m = 50;
    QVector<QPair<double, double> > cssn;
    for(int i=0;i<m;i++) {
        double phi = M_PI * 2 * i / (m-1);
        cssn.append(qMakePair(cos(phi)*r, sin(phi)*r));
    }
    glColor3d(0.5,0.5,0.5);    
    glBegin(GL_LINE_STRIP);
    for(int i=0;i<m;i++) glVertex3d(cssn[i].first,cssn[i].second,0);
    glEnd();
    glBegin(GL_LINE_STRIP);
    for(int i=0;i<m;i++) glVertex3d(cssn[i].first,0,cssn[i].second);
    glEnd();
    glBegin(GL_LINE_STRIP);
    for(int i=0;i<m;i++) glVertex3d(0,cssn[i].first,cssn[i].second);
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
#include "Viewer.h"
#include "Gutil.h"
#include "Mesh.h"
#include "Page.h"
#include "Overlay.h"


#include <QPainter>
#include <QDebug>
#include <QGLShaderProgram>
#include <QGLShader>
#include <qfile.h>
#define _USE_MATH_DEFINES
#include <math.h>


#include <assert.h>
#include <QKeyEvent>
#include <QWheelEvent>
#include <QGLShaderProgram>
#include <qmatrix4x4.h>
#include <qvector.h>
#include <qlist.h>

GLuint texid;

Viewer::Viewer()
: QGLWidget()
, m_fps(0)
{  
    setFocusPolicy(Qt::ClickFocus);
    m_presentation = new Presentation(this);
    m_presentation->buildPages();
    m_overlay = new Overlay(this);
}

Viewer::~Viewer()
{
    delete m_presentation;
    delete m_overlay;
}

QSize Viewer::sizeHint() const
{
    return QSize(1024,768);
}

void Viewer::initializeGL()
{
  // glEnable(GL_MULTISAMPLE);
  //glClearColor(0.2,0.3,0.4,1);
  glClearColor(1,1,1,1);
  // QPainter painter(this);
  // painter.setRenderHints(QPainter::HighQualityAntialiasing);
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
  m_presentation->initializeGL();


  texid = bindTexture("images/cubic-space-division.jpg");
  glBindTexture(GL_TEXTURE_2D, 0);


  /*
  QImage img("images/cubic-space-division.jpg");
  int w = img.width();
  int h = img.height();


  //------------------------------
   glGenTextures(1, &texid);

   QImage img2 = img.convertToFormat(QImage::Format_ARGB32);
   glBindTexture(GL_TEXTURE_2D, texid);
   gluBuild2DMipmaps( GL_TEXTURE_2D, 4, img.width(), img.height(),
                   GL_BGRA_EXT, GL_UNSIGNED_BYTE, img.bits() );
  //glTexImage2D(GL_TEXTURE_2D, 0, 4, image.width(), image.height(), 0,
  //  GL_BGRA, GL_UNSIGNED_BYTE, image.bits());
  */
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
  //glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
  //glTexParameteri(GL_TEXTURE_2D, GL_GENERATE_MIPMAP, GL_TRUE);
  glBindTexture(GL_TEXTURE_2D, 0);
  //.--------------------------------
  glGenTextures(1, &m_textTextureId);
  m_overlay->initializeGL();

  startTimer(20);
  m_clock.start();
}

void Viewer::resizeGL(int width, int height)
{
    m_presentation->resizeGL(width, height);

    /*
  glViewport(0,0,width,height);
  glMatrixMode(GL_PROJECTION);
  glLoadIdentity();
  glOrtho(0,width,0,height,-1,1);
  glMatrixMode(GL_MODELVIEW);
  */

    /*

 double aspect = (float)width/height;
    glViewport(0,0,width,height);
    glMatrixMode(GL_PROJECTION);
    glLoadIdentity();
    gluPerspective(45, aspect, 1.0, 70.0);
    glMatrixMode(GL_MODELVIEW);
    glLoadIdentity();
    */
}

void Viewer::paintGL()
{
    int err;
    err = glGetError() ;
    assert(err == GL_NO_ERROR);
    m_presentation->paintGL();
    /*
    glClearColor(0,0,0,1);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    
    drawBackground();

    glPushMatrix();
    glTranslated(0,0,-10);
    glRotated(20,1,0,0);
    glRotated(30,0,1,0);

    drawAxes();

    m_presentation->p()->paintGL();
    glPopMatrix();
    */
    err = glGetError() ;
    assert(err == GL_NO_ERROR);

    qglColor(Qt::cyan);
    renderText(50,50,QString::number((int)m_fps), QFont("Calibri", 24));

    m_overlay->draw(size());
    


    



    int ms = m_clock.restart();
    if(ms>0)
    {
        double fps = 1000.0/(double)ms;
        m_fps = 0.9 * m_fps + 0.1 * fps;
    }

}


QGLShaderProgram *Viewer::loadProgram(QString name)
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


void Viewer::drawBackground()
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

void Viewer::drawAxes(double r)
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


void Viewer::mousePressEvent(QMouseEvent *e)
{
    m_presentation->p()->mousePressEvent(e);
}

void Viewer::mouseMoveEvent(QMouseEvent *e)
{
    m_presentation->p()->mouseMoveEvent(e);
}

void Viewer::mouseReleaseEvent(QMouseEvent *e)
{
    m_presentation->p()->mouseReleaseEvent(e);
}

/*
void Viewer::showEvent(QShowEvent *)
{
    m_presentation->p()->showEvent(e);
}
*/


void Viewer::keyPressEvent(QKeyEvent *e)
{
    if(e->key() == Qt::Key_PageDown 
        || e->key() == Qt::Key_Down)
    {
        m_presentation->nextPage();
    }
    else if(e->key() == Qt::Key_PageUp
        || e->key() == Qt::Key_Up)
    {
        m_presentation->prevPage();
    }
    else if(e->key() == Qt::Key_Escape)
    {
        close();
    }
    else if(e->key() == Qt::Key_F11)
    {
        if(isFullScreen())
            setWindowState(windowState() & ~Qt::WindowFullScreen);
        else
            setWindowState(windowState() | Qt::WindowFullScreen);
        setFocus();
    }
  else
    m_presentation->p()->keyPressEvent(e);
}

void Viewer::wheelEvent(QWheelEvent*e)
{
    m_presentation->p()->wheelEvent(e);
}

void Viewer::timerEvent(QTimerEvent*)
{
    updateGL();
}



void Viewer::drawText(const QVector3D &pos, const QString &text, double r, const QColor &color)
{
    GLdouble viewArr[16];
    glGetDoublev(GL_MODELVIEW_MATRIX, viewArr);
    
    QVector3D du = QVector3D(viewArr[0], viewArr[4], viewArr[8]) * r;
    QVector3D dv = QVector3D(viewArr[1], viewArr[5], viewArr[9]) * r * 0.5;
    QVector3D pp[4] = {pos-du-dv, pos+du-dv, pos+du+dv, pos-du+dv};
    GLfloat ppc[4][3];
    for(int i=0;i<4;i++) {const QVector3D &p = pp[i]; ppc[i][0] = p.x(); ppc[i][1] = p.y(); ppc[i][2] = p.z(); }
     
    QImage img(64,32,QImage::Format_ARGB32_Premultiplied);
    img.fill(Qt::transparent);
    // img.fill(QColor(200,200,200,127));
    QPainter pa;
    pa.begin(&img);
    pa.setFont(QFont("Arial", 20));
    pa.setPen(color);
    pa.drawText(QRect(0,0,img.width(),img.height()), Qt::AlignCenter, text);
    pa.drawRect(0,0,img.width()-1, img.height()-1);
    pa.end();
    QImage glImg = img.convertToFormat(QImage::Format_ARGB32);
    glBindTexture(GL_TEXTURE_2D, m_textTextureId);
    gluBuild2DMipmaps( GL_TEXTURE_2D, 4, glImg.width(), glImg.height(),
                   GL_BGRA_EXT, GL_UNSIGNED_BYTE, glImg.bits() );
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);

    glDisable(GL_LIGHTING);
    glEnable(GL_TEXTURE_2D);
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA,GL_ONE_MINUS_SRC_ALPHA);
    glBegin(GL_QUADS);
    glTexCoord2f(0.0f, 1.0f); glVertex3fv(ppc[0]);
    glTexCoord2f(1.0f, 1.0f); glVertex3fv(ppc[1]);
    glTexCoord2f(1.0f, 0.0f); glVertex3fv(ppc[2]);
    glTexCoord2f(0.0f, 0.0f); glVertex3fv(ppc[3]);
    glEnd();
    glDisable(GL_BLEND);
    glDisable(GL_TEXTURE_2D);
    glBindTexture(GL_TEXTURE_2D, 0);
    glEnable(GL_LIGHTING);
    
}

#include "JohnsonSolidsPage.h"


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
#include <QPainter>


JohnsonSolidsPage::JohnsonSolidsPage()
: m_cameraDistance(15)
, m_status(-1)
{
}

JohnsonSolidsPage::~JohnsonSolidsPage()
{
}

void JohnsonSolidsPage::initializeGL()
{
}

void JohnsonSolidsPage::start()
{
  glEnable(GL_LIGHTING);
  glEnable(GL_DEPTH_TEST);
  glEnable(GL_LIGHT0);
  // glEnable(GL_LIGHT1);
  glLightModelf(GL_LIGHT_MODEL_TWO_SIDE,1.0);
  glLightModelf(GL_LIGHT_MODEL_LOCAL_VIEWER,1.0);

  createTextures();
  setStatus(0);
}

void JohnsonSolidsPage::stop()
{
    destroyTextures();
}

void JohnsonSolidsPage::resizeGL(int width, int height)
{
    double aspect = (float)width/height;
    glViewport(0,0,width,height);
    glMatrixMode(GL_PROJECTION);
    glLoadIdentity();
    gluPerspective(45, aspect, 1.0, 200.0);
    glMatrixMode(GL_MODELVIEW);
    glLoadIdentity();
}


void JohnsonSolidsPage::paintGL()
{
    glClearColor(0,0,0,1);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    drawBackground();

    GLfloat specular[] =  { 0.7f, 0.7f, 0.7f, 1.0f};
    glMaterialfv(GL_FRONT_AND_BACK, GL_SPECULAR, specular);
    glMaterialf(GL_FRONT_AND_BACK, GL_SHININESS, 90.0);

    setColor(1,1,1);
}


void JohnsonSolidsPage::keyPressEvent(QKeyEvent *e)
{
    if(e->key() == Qt::Key_Right) setStatus(m_status+1);
    else if(e->key() == Qt::Key_Left) setStatus(m_status-1);
    e->ignore();
    updateGL();
}


void JohnsonSolidsPage::createTextures()
{
    QImage img;
    QPainter pa;
    OverlayPanel *panel;

    // wikipage
    panel = &m_panels.wikipage;
    panel->createTexture(QImage("images/johnson_wikipage.png"));
    panel->setSize(90);
    panel->setPosition(0.5,0.5);

    // polyhedra
    panel = &m_panels.polyhedra;
    panel->createTexture(QImage("images/poliedri.jpg"));
    panel->setSize(90);
    panel->setPosition(0.5,0.5);

    // polyhedra
    panel = &m_panels.xyz;
    panel->createTexture(QImage("images/xyz.png"));
    panel->setSize(90);
    panel->setPosition(0.5,0.5);
    

}

void JohnsonSolidsPage::destroyTextures()
{
    OverlayPanel *panels[] = {
        &m_panels.wikipage,
        &m_panels.polyhedra,
        &m_panels.xyz,
    };
    for(int i=0;i<sizeof(panels)/sizeof(panels[0]);i++) panels[i]->deleteTexture();
}

void JohnsonSolidsPage::setStatus(int status)
{
    m_status = status;
    Overlay *ov = getOverlay();
    ov->removeAll();
    if(status == 0) ov->add(&m_panels.wikipage);
    else if(status == 1) ov->add(&m_panels.polyhedra);
    else if(status == 2) ov->add(&m_panels.xyz);  
}

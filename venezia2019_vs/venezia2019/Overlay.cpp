#include "Overlay.h"
#include "Gutil.h"

#include "Viewer.h"
#include <QGLWidget>

Overlay::Overlay(Viewer *viewer)
    : m_viewer(viewer)
{
}

Overlay::~Overlay()
{
}

void Overlay::initializeGL()
{
    m_clock.start();
}


void Overlay::add(OverlayPanel *panel)
{
    m_panels.append(panel);
}

void Overlay::remove(OverlayPanel *panel)
{
    m_panels.removeAll(panel);
}


void Overlay::draw(int width, int height)
{
    if(m_panels.empty()) return;
    double time = m_clock.elapsed() * 0.001;


    m_viewer->qglColor(Qt::magenta);
    m_viewer->renderText(50,100,"Hyperbolic honeycomb", QFont("Calibri", 60, QFont::Bold));
    m_viewer->renderText(50,200,"Gian Marco Todesco", QFont("Calibri", 30));

    /*

    glMatrixMode(GL_PROJECTION);
    glPushMatrix();
    glLoadIdentity();
    glOrtho(0,width(),0,height(),-1,1);
    glMatrixMode(GL_MODELVIEW);
    glPushMatrix();
    glLoadIdentity();
    

    glDisable(GL_LIGHTING);
    glColor3d(1,1,0);

    glEnable(GL_TEXTURE_2D);
    glTexEnvi(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_MODULATE);
    glBindTexture(GL_TEXTURE_2D, texid);
    glBegin(GL_QUADS);
    glTexCoord2f(0.0f, 0.0f); glVertex2i(100,100);
    glTexCoord2f(1.0f, 0.0f); glVertex2i(400,100);
    glTexCoord2f(1.0f, 1.0f); glVertex2i(400,400);
    glTexCoord2f(0.0f, 1.0f); glVertex2i(100,400);
    glEnd();
    glBindTexture(GL_TEXTURE_2D, 0);
    glDisable(GL_TEXTURE_2D);
    

    glEnable(GL_LIGHTING);


    glPopMatrix();
    glMatrixMode(GL_PROJECTION);
    glPopMatrix();
    glMatrixMode(GL_MODELVIEW);
    
*/

}

OverlayPanel::OverlayPanel(const QString &filename)
{
    QImage img(filename);

}
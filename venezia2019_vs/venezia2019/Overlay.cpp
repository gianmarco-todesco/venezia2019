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
    if(!m_panels.contains(panel))
        m_panels.append(panel);
}

void Overlay::remove(OverlayPanel *panel)
{
    m_panels.removeAll(panel);
}

void Overlay::removeAll()
{
    m_panels.clear();
}


void Overlay::draw(const QSize &winSize)
{
    if(m_panels.empty()) return;
    int width = winSize.width();
    int height = winSize.height();

    double time = m_clock.elapsed() * 0.001;

    // set window coords
    glMatrixMode(GL_PROJECTION);
    glPushMatrix();
    glLoadIdentity();
    glOrtho(0,width,0,height,-1,1);
    glMatrixMode(GL_MODELVIEW);
    glPushMatrix();
    glLoadIdentity();

    glDisable(GL_LIGHTING);
    glDisable(GL_DEPTH_TEST);
    glColor3d(1,1,1);

    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA,GL_ONE_MINUS_SRC_ALPHA);

    /*

    m_viewer->qglColor(Qt::magenta);
    m_viewer->renderText(50,100,"Hyperbolic honeycomb", QFont("Calibri", 60, QFont::Bold));
    m_viewer->renderText(50,200,"Gian Marco Todesco", QFont("Calibri", 30));
    */

   

    glEnable(GL_TEXTURE_2D);
    glTexEnvi(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_MODULATE);

    foreach(OverlayPanel *panel, m_panels)
    {
        panel->draw(winSize);
    }

    glDisable(GL_TEXTURE_2D);
    glDisable(GL_BLEND);
    

    glEnable(GL_LIGHTING);
    glEnable(GL_DEPTH_TEST);

    // reset coords
    glPopMatrix();
    glMatrixMode(GL_PROJECTION);
    glPopMatrix();
    glMatrixMode(GL_MODELVIEW);
    

}

//=============================================================================


OverlayPanel::OverlayPanel()
    : m_textureId(0)
    , m_size(90)
    , m_pos(0.5,0.5)
    , m_hasAlpha(false)
{
}


void OverlayPanel::createTexture(const QImage &img)
{
    QImage glImg = img.convertToFormat(QImage::Format_ARGB32);
    m_width = img.width();
    m_height = img.height();
    glGenTextures(1, &m_textureId);
    glBindTexture(GL_TEXTURE_2D, m_textureId);
    gluBuild2DMipmaps( GL_TEXTURE_2D, 4, m_width, m_height,
                   GL_BGRA_EXT, GL_UNSIGNED_BYTE, glImg.bits() );
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    //glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
    //glTexParameteri(GL_TEXTURE_2D, GL_GENERATE_MIPMAP, GL_TRUE);
    glBindTexture(GL_TEXTURE_2D, 0);
}

OverlayPanel::~OverlayPanel()
{
    if(m_textureId!=0) deleteTexture();
}


void OverlayPanel::deleteTexture()
{
    glDeleteTextures(1, &m_textureId);
    m_textureId = 0;
    m_width = m_height = 0;
}


void OverlayPanel::draw(const QSize &winSize)
{
    if(m_textureId==0) return;
    double sz = m_size * 0.01 * winSize.height();
    double ry = 0.5 * sz;
    double rx = ry * m_width / m_height;

    double x = winSize.width() * m_pos.x();
    double y = winSize.height() * m_pos.y();

    glBindTexture(GL_TEXTURE_2D, m_textureId);
    glBegin(GL_QUADS);
    glTexCoord2f(0.0f, 1.0f); glVertex2d(x-rx,y-ry);
    glTexCoord2f(1.0f, 1.0f); glVertex2d(x+rx,y-ry);
    glTexCoord2f(1.0f, 0.0f); glVertex2d(x+rx,y+ry);
    glTexCoord2f(0.0f, 0.0f); glVertex2d(x-rx,y+ry);
    glEnd();
    glBindTexture(GL_TEXTURE_2D, 0);
}

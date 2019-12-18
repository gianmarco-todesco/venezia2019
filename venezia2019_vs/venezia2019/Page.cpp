#include "Page.h"
#include "Viewer.h"
#include "Gutil.h"

#include "DummyPage.h"
#include "PDiskPage.h"
#include "FoldingFacesPage.h"
#include "CircleLimit3Page.h"
#include "CubeGridPage.h"
#include "PolydronPage.h"
#include "H3GridPage.h"
#include "H3GridBuildPage.h"
#include "ImpossiblePolyhedronPage.h"
#include "HyperbolicPolyhedronPage.h"
#include "JohnsonSolidsPage.h"
#include "TestPage.h"

#include "Fig1Page.h"
#include "Fig2Page.h"
#include "Fig3Page.h"
#include "Fig4Page.h"
#include "Fig5Page.h"
#include "Fig6Page.h"
#include "Fig7aPage.h"
#include "Fig7bPage.h"
#include "Fig8aPage.h"
#include "Fig8bPage.h"
#include "Fig9Page.h"
#include "Fig10Page.h"
#include "Fig11Page.h"
#include "Fig12Page.h"

#include <qglshaderprogram.h>

#include <assert.h>


void Presentation::buildPages()
{

    addPage(new Fig1Page());
    
    // addPage(new Fig11Page());
    // addPage(new Fig12Page());
    
    /*
    // addPage(new H3GridBuildPage());
    
    addPage(new Fig1Page());
    addPage(new Fig2Page());
    addPage(new Fig3Page());
    addPage(new Fig4Page());
    addPage(new Fig5Page());
    addPage(new Fig6Page());
    addPage(new Fig7aPage());
    addPage(new Fig7bPage());
    addPage(new Fig8aPage());
    addPage(new Fig8bPage());
    addPage(new Fig9Page());
    addPage(new Fig10Page());
    addPage(new Fig11Page());
    addPage(new Fig12Page());
    */


    /*
    addPage(new H3GridPage());
    

    addPage(new CubeGridPage());
    addPage(new PolydronPage());
    addPage(new JohnsonSolidsPage());
    addPage(new ImpossiblePolyhedronPage());
    addPage(new CircleLimit3Page());   
    addPage(new PDiskPage());
    addPage(new FoldingFacesPage());
    addPage(new HyperbolicPolyhedronPage());
    addPage(new H3GridBuildPage());
    addPage(new H3GridPage());
    */


}

//=============================================================================


Page::Page()
    : m_viewer(0)
    , m_width(0)
    , m_height(0)
{
}

Page::~Page()
{
}

void Page::setSize(int width, int height) 
{
    m_width = width;
    m_height = height;
}

void Page::drawBackground()
{
    m_viewer->drawBackground();
}

void Page::drawAxes()
{
    m_viewer->drawAxes();
}

QGLShaderProgram *Page::loadProgram(QString name)
{
    return m_viewer->loadProgram(name);
}

void Page::updateGL()
{
    m_viewer->updateGL();
}

Overlay *Page::getOverlay() const
{
    return m_viewer->getOverlay();
}

void Page::setViewUniforms(QGLShaderProgram*program)
{
    GLdouble viewArr[16];
    glGetDoublev(GL_MODELVIEW_MATRIX, viewArr);
    QMatrix4x4 view(viewArr);
    QMatrix4x4 mview;

    program->setUniformValue("view", view);
    program->setUniformValue("mview", mview);
    program->setUniformValue("iview", view.inverted());

}

#define GL_MULTISAMPLE 0x809D


void Page::savePicture(const QString &path, int border)
{
    assert(glGetError() == GL_NO_ERROR);
    QGLFormat fmt;
    
    // fmt.setDoubleBuffer(false);
    // fmt.setAlpha(false);
        
    // fmt.setSampleBuffers(true);
    fmt.setDepth(true);

    int width = 4096, height = 4096;
    // width=height=1024;
    QGLPixelBuffer buffer(width,height,fmt);    
    buffer.makeCurrent();

    setSize(width, height);
    initializeGL();
    resizeGL(width, height);
   
    paintGL();
    
    glFlush();
    glFinish();
    stop();

    buffer.doneCurrent();
    QImage img = buffer.toImage();
    
    if(border>0)
    {
        QPainter pa;
        pa.begin(&img);
        QBrush color = Qt::black;
        pa.fillRect(0,0,img.width(),border, color);
        pa.fillRect(0,img.height()-border,img.width(),border, color);
        pa.fillRect(0,0,border,img.height(), color);
        pa.fillRect(img.width()-border,0,border,img.height(), color);
        pa.end();
    }
    img.save(path);
    assert(glGetError() == GL_NO_ERROR);

}



//=============================================================================

Presentation::Presentation(Viewer *viewer)
    : m_viewer(viewer)
    , m_width(0)
    , m_height(0)
    , m_started(false)
{
    m_currentPageIndex = -1;
    m_currentPage = &m_emptyPage;
}

Presentation::~Presentation()
{
    foreach(Page*page, m_pages) delete page;
    m_pages.clear();
}

void Presentation::initializeGL()
{
    foreach(Page*page, m_pages) page->initializeGL();
}

void Presentation::resizeGL(int width, int height)
{
    m_width = width;
    m_height = height;
    m_currentPage->setSize(width, height);
    m_currentPage->resizeGL(width, height);
}

void Presentation::paintGL()
{
    if(!m_started) { m_started=true; p()->start(); }
    p()->paintGL();
}


void Presentation::addPage(Page*page)
{
    page->setViewer(m_viewer);
    m_pages.append(page);
    if(m_currentPageIndex<0) { m_currentPageIndex = 0; m_currentPage = page; }
}


void Presentation::start(Page *page)
{
    page->setSize(m_width, m_height);
    page->resizeGL(m_width, m_height);
    page->start();
}

void Presentation::stop(Page *page)
{
    m_viewer->getOverlay()->removeAll();
    page->stop();
}


void Presentation::setPage(int index)
{
    m_currentPageIndex = index;
    Page *newPage = 0<=index && index<m_pages.count() ? m_pages[index] : &m_emptyPage;
    if(newPage != m_currentPage)
    {
        if(m_currentPage) stop(m_currentPage);
        m_currentPage = newPage;
        if(m_currentPage) start(m_currentPage);
        m_viewer->updateGL();
    }
}

void Presentation::nextPage()
{
    if(m_currentPageIndex>=0 && m_currentPageIndex+1<m_pages.count()) 
    {
        setPage(m_currentPageIndex+1);
    }
}

void Presentation::prevPage()
{
    if(m_currentPageIndex>0) setPage(m_currentPageIndex-1);
}


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
#include "ImpossiblePolyhedronPage.h"
#include "HyperbolicPolyhedronPage.h"
#include "JohnsonSolidsPage.h"
#include "TestPage.h"


#include <qglshaderprogram.h>


void Presentation::buildPages()
{
    addPage(new PolydronPage());

    addPage(new JohnsonSolidsPage());
    addPage(new CubeGridPage());
    addPage(new TestPage());

    addPage(new HyperbolicPolyhedronPage());
    addPage(new ImpossiblePolyhedronPage());
    


    addPage(new H3GridPage());
    // 
    //addPage(new CircleLimit3Page());
    //addPage(new PDiskPage());
    // addPage(new FoldingFacesPage());
    /*
    addPage(new DummyPage());
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


//=============================================================================


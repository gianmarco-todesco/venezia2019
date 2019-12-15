#include "Fig7bPage.h"

#define _USE_MATH_DEFINES
#include <math.h>
#include <vector>

#include <assert.h>
#include <QKeyEvent>
#include <QWheelEvent>

#include "Gutil.h"
#include "Point3.h"
#include "Polyhedra.h"
#include "H3.h"
#include "H3Grid.h"
#include "CircleLimit3Viewer.h"

#include <QGLShaderProgram>
#include <qmatrix4x4.h>
#include <QVector4D.h>
#include <qvector.h>
#include <qlist.h>
#include <QGLBuffer>
#include <QTIme>


#include "Viewer.h"

// fig.7 (a) A regular Hyperbolic tessellation: {8,3} (b) the structure of Escher's "Circle Limit III"


Fig7bPage::Fig7bPage(bool figureMode)
    : CircleLimit3Page(figureMode)
{
}
    
Fig7bPage::~Fig7bPage()
{
}



 void Fig7bPage::paintGL()
 {
     glClearColor(1,1,1,1);
     glClear(GL_COLOR_BUFFER_BIT);

    int w = width(), h = height();

    m_viewer->resize(width(), height(), height()*0.45);
    m_viewer->m_status = 0;
    glColor4d(1,1,1,1);
    m_viewer->draw();

    double unit = w * 0.001;

    glScissor(0,0,w/2,h);
    glEnable(GL_SCISSOR_TEST);

    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
    glColor4d(0.8,0.8,0.8,0.8);
    fillCircle(m_viewer->getPan(), m_viewer->getScale(), 200);
    glDisable(GL_BLEND);

    glColor3d(0,0,0);
    m_viewer->drawTess2(unit * 3.3);
    glColor3d(1,1,1);
    m_viewer->drawTess2(unit * 3);
    glColor3d(0,0,0);
    m_viewer->drawTess2(unit * .5);
    glDisable(GL_SCISSOR_TEST);
    glScissor(0,0,w,h);
    glColor3d(0,0,0);
    double x = width()*0.5;
    glRectd(x-5*unit,0,x+5*unit,height());
 }



void Fig7bPage::savePictures()
{
    Fig7bPage page(true);
    page.savePicture("fig7b.png");
    // savePicture("fig7b.png");
}

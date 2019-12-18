#include "Fig8bPage.h"

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

#include <QGLShaderProgram>
#include <qmatrix4x4.h>
#include <QVector4D.h>
#include <qvector.h>
#include <qlist.h>
#include <QGLBuffer>
#include <QTIme>


#include "Viewer.h"

// fig.8 Two groups of Hyperbolic octagons: (b) larger size: the octagons fit nicely


using namespace GmLib;

const double pi = 3.1415926;


Fig8bPage::Fig8bPage()
{
}

Fig8bPage::~Fig8bPage()
{
}

    
void Fig8bPage::savePictures()
{
    Fig8bPage page;

    page.savePicture("Fig8b.png");
}

void Fig8bPage::resizeGL(int width, int height)
{
    PDiskPage::resizeGL(width, height);
    m_pan = QPointF(width*0.5, height*0.5);
    m_scale = height*0.45;
}


    
void Fig8bPage::paintGL()
{
    glClear(GL_COLOR_BUFFER_BIT);
    // glEnable(GL_MULTISAMPLE);
  
    // disk background
    glColor3d(1,1,1);
    double r = m_scale;
    const QPointF center = m_pan;


    fillCircle(center,r,250);

    // ottagono con due vicini
    int n = 8; // , m = 3;
    double phi = 2*pi/n;
    
    // Complex c = m_pts[0];
    Complex c(-0.24074074074074073, 0.32716049382716050);

    Complex cphi = Complex(cos(phi),sin(phi));
    Complex c0 = cphi * c;
    Complex c1 = cphi.conj() * c;
    HLine s0 = HLine::makeSegment(c,c0);
    HLine s1 = HLine::makeSegment(c,c1);
    double psi = acos(dot(s0.getDir(0),s1.getDir(0)));

    drawPolygon(c, n);

    pushTransform();
    HTransform tr = HTransform::translation(c) * HTransform::rotation(psi) * HTransform::translation(-c);
    transform(tr);
    drawPolygon(c, n);
    popTransform();

    pushTransform();
    tr = HTransform::translation(c) * HTransform::rotation(-psi) * HTransform::translation(-c);
    transform(tr);
    drawPolygon(c, n);
    popTransform();


    // contorno del disco
    glColor3d(0,0,0);
    int m = 100;
    glBegin(GL_TRIANGLE_STRIP);
    double r1 = r + 4;
    for(int i=0;i<=m;i++)
    {
        double phi = M_PI*2*i/m;
        double cs = cos(phi), sn = sin(phi);
        glVertex2d(center.x() + r*cs, center.y() + r*sn);
        glVertex2d(center.x() + r1*cs, center.y() + r1*sn);
    }
    glEnd();
    


}


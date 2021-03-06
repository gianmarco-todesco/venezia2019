#include "H2TessellationPage.h"

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

// fig.2 A regular tessellation of the Hyperbolic plane represented on the Poincar� disk model. 
// The tessellation is named {5,4}: four pentagons meet at each vertex.


using namespace GmLib;

const double pi = 3.1415926;


H2TessellationPage::H2TessellationPage(int n, int q, bool veryDetailed)
    : m_n(n)
    , m_q(q)
    , m_veryDetailed(veryDetailed)
{
}

H2TessellationPage::~H2TessellationPage()
{
}


void H2TessellationPage::resizeGL(int width, int height)
{
    PDiskPage::resizeGL(width, height);
    m_pan = QPointF(width*0.5, height*0.5);
    m_scale = height*0.45;
}


    
void H2TessellationPage::paintGL()
{
    glClearColor(1,1,1,1);
    glClear(GL_COLOR_BUFFER_BIT);
    // glEnable(GL_MULTISAMPLE);
  
    // disk background
    glColor3d(1,1,1);
    double r = m_scale;
    const QPointF center = m_pan;


    fillCircle(center,r,250);

    // int n = 5, m = 4;
    int n = m_n;
    int m = m_q;
    if(m_tess == 0 || m_tess->getN()!=n || m_tess->getM()!=m)
    {
        delete m_tess;
        m_tess = new HTessellation(n,m);
        m_tess->init(m_veryDetailed ? 5000 : 2000);
    }

    for(int i=0;i<m_tess->getFaceCount();i++)
    {
      HTransform tr = m_tess->getFaceTransform(i);
      pushTransform();
      transform(tr);
      Complex c = m_transformStack.back()*Complex(0,0);
      double v = 0.75 + 0.25 * c.getNorm();
      glColor3d(v,v,v);
      for(int j=0;j<n;j++)
      {
        glBegin(GL_TRIANGLE_FAN);
        glVertex(diskToWorld(c));
        for(int kk=0;kk<20;kk++)
        {
          double t = (double)kk/19.0;
          glVertex(diskToWorld(m_transformStack.back()*m_tess->border(j,t)));
        }
        glEnd();
      }
      popTransform();
    }


    for(int i=0;i<m_tess->getEdgeCount();i++)
    {
      Net::HEdge *he = m_tess->getEdge(i)->getHedge();
      Net::Face *f = he->getLeftFace(); if(f==0) f = he->getRightFace();
      HTransform tr = m_tess->getFaceTransform(f->getIndex());
      int j = m_tess->my(he)->indexInFace;
      pushTransform();
      transform(tr);
      drawHLine(
          HLine::makeSegment(
              m_transformStack.back()*m_tess->border(j,0),
              m_transformStack.back()*m_tess->border(j,1)),
          Color(0,0,0));
      popTransform();
    }


  // contorno del disco
  glLineWidth(4);
  glColor3d(0,0,0);
  drawCircle(center,r,200);
  glLineWidth(1);


}


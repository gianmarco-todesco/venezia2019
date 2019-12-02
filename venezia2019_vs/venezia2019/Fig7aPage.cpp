#include "Fig7aPage.h"

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

// fig. 7a: A regular Hyperbolic tessellation: {8,3}

using namespace GmLib;

const double pi = 3.1415926;


Fig7aPage::Fig7aPage()
{
}

Fig7aPage::~Fig7aPage()
{
}

    
void Fig7aPage::savePictures()
{
    Fig7aPage page;

    page.savePicture("fig7a.png");
}

void Fig7aPage::resizeGL(int width, int height)
{
    PDiskPage::resizeGL(width, height);
    m_pan = QPointF(width*0.5, height*0.5);
    m_scale = height*0.45;
}


    
void Fig7aPage::paintGL()
{
    glClear(GL_COLOR_BUFFER_BIT);
    // glEnable(GL_MULTISAMPLE);
  
    // disk background
    glColor3d(1,1,1);
    double r = m_scale;
    const QPointF center = m_pan;


    fillCircle(center,r,250);

    int n = 8, m = 3;
    if(m_tess == 0 || m_tess->getN()!=n || m_tess->getM()!=m)
    {
        delete m_tess;
        m_tess = new HTessellation(n,m);
        m_tess->init(500);
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


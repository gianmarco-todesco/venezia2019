#include "PDiskPage.h"
#include "Gutil.h"
#include <QMouseEvent>
#include <QKeyEvent>
#include <QWheelEvent>
#include <math.h>
#include <QGLPixelBuffer>
#include <QPainter>
#include <QPainterPath>
#include <QApplication>

using namespace GmLib;

const double pi = 3.1415926;

Complex HLine::getPoint(double t) const
{
  Complex p = (1-t)*m_pa + t*m_pb;
  if(m_radius!=0.0)
  {
    p = m_center + m_radius * (p-m_center).normalized();
  }
  return p;
}

Complex HLine::getDir(double t) const
{
  if(m_radius==0.0) return (m_pb-m_pa).normalized();
  Complex d = Complex(0,1) * (getPoint(t)-m_center).normalized();
  if(dot(m_pb-m_pa, d)<0) d= -d;
  return d;
}

HLine HLine::makeSegment(const Complex & pa, const Complex & pb)
{
  HLine hline;
  hline.m_pa = pa;
  hline.m_pb = pb;
  double dsc = pa.re*pb.im-pa.im*pb.re;
  if(fabs(dsc)<0.0001)
  {
    hline.m_center = 0.5*(pa+pb);
    hline.m_radius = 0;
  }
  else
  {
    double a2 = pa.getNorm2();
    double b2 = pb.getNorm2();
    double cx = -0.5*(pa.im*b2-pb.im*a2+pa.im-pb.im)/dsc;
    double cy = -0.5*(pb.re*a2-pa.re*b2+pb.re-pa.re)/dsc;
    hline.m_center = Complex(cx,cy);
    hline.m_radius = sqrt(cx*cx+cy*cy-1);
  }
  return hline;
}

HLine HLine::makeLine(const Complex & pa, const Complex & pb)
{
  HLine line = makeSegment(pa,pb);
  if(line.m_radius == 0)
  {
    double a2 = pa.getNorm2(), b2 = pb.getNorm2();
    double p2;
    Complex p;
    if(a2>b2) {p2=a2;p=pa;} else {p2=b2;p=pb;}
    if(p2<0.00001)
    {
      line.m_pa = Complex(1,0);
      line.m_pb = Complex(-1,0);
    }
    else
    {
      p = p * (1.0/sqrt(p2));
      if(dot(p,pa)>dot(p,pb)) {line.m_pa = p; line.m_pb = -p; }
      else {line.m_pb = p; line.m_pa = -p; }
    }
  }
  else
  {
    double d = line.m_center.getNorm();
    Complex u = (1.0/d) * line.m_center;
    Complex w = Complex(0,1) * u;
    double q = 1/d, h = q * line.m_radius;
    Complex p0 = u*q+w*h, p1 = u*q-w*h;
    line.m_pa = p0;
    line.m_pb = p1;
  }
  return line;
}

//=============================================================================


PDiskPage::PDiskPage()
: m_status(0)
, m_oldStatus(-1)
, m_scale(250)
, m_pan(300,300)
, m_currentPoint(-1)
, m_panning(false)
, m_drawing(false)
, m_repeatDrawings(false)
, m_tess(0)
{
  setFocusPolicy(Qt::ClickFocus);
  m_transformStack.push_back(HTransform());
  m_pts.push_back(Complex(-0.4,0.6));
  m_pts.push_back(Complex(0.4,0.3));
  m_pts.push_back(Complex(-0.2,-0.7));
  m_pts.push_back(Complex(0.1,0.1));// 3
  m_pts.push_back(Complex(0.1,-0.8));
}

void PDiskPage::showEvent(QShowEvent *)
{
  setFocus();
  m_pan = QPointF(width()*0.5, height()*0.5);
  m_scale = height()*0.45;
}

Complex PDiskPage::boundPoint(const Complex &c) const
{
  double d2 = c.getNorm2();
  if(d2<1) return c;
  else return c * (1.0/sqrt(d2));
}

double PDiskPage::getThickness(const Complex &c) const
{
  double r2 = c.getNorm2();
  if(r2>1) return 0;
  else return 1-r2;
}

void PDiskPage::drawPoint(const Complex &c, double rFactor) const
{
  Complex cc = m_transformStack.back() * c;
  QPointF p = diskToWorld(cc);
  double r = rFactor*(1 + 2*getThickness(cc));
  if(r>4)r=4;
  fillCircle(p,r,10);
  glColor3d(0,0,0);
  if(rFactor>1)
    glLineWidth(2);
  drawCircle(p,r,10);
  if(rFactor>1)
    glLineWidth(0);
}

void PDiskPage::drawHLine(const HLine &hline, const Color &color) const
{
  Complex c0 = hline.getPoint(0);
  Complex c1 = hline.getPoint(1);
  int m = 2 + (int)((c0-c1).getNorm()*(50+150));
  glBegin(GL_TRIANGLE_STRIP);
  for(int i=0;i<m;i++)
  {
    double t = i/(double)(m-1);
    Complex c = hline.getPoint(t);
    double thickness = 0.4+1.5*getThickness(c);
    Complex dir = thickness * hline.getDir(t);
    QPointF d(-dir.im, dir.re);
    QPointF p = diskToWorld(c);
    double v = 0.5*c.getNorm();
    glColor3d(color.r+(1-color.r)*v,color.g+(1-color.g)*v,color.b+(1-color.b)*v);
    glVertex(p+d);
    glVertex(p-d);
  }
  glEnd();
}

void PDiskPage::drawSegment(const Complex &c0, const Complex &c1) const
{
  const HTransform &tr = m_transformStack.back();
  drawHLine(HLine::makeSegment(tr*c0,tr*c1), Color(0,0,0));
}

void PDiskPage::drawLine(const Complex &c0, const Complex &c1, const Color &color) const
{
  const HTransform &tr = m_transformStack.back();
  drawHLine(HLine::makeLine(tr*c0,tr*c1), color);
}

void PDiskPage::drawPolygon(const Complex &c0, int n) const
{
  std::vector<Complex> pts(n);
  for(int i=0;i<n;i++)
  {
    double phi = 2*pi*i/n;
    pts[i] = Complex(cos(phi),sin(phi)) * c0;
  }
  for(int i=0;i<n;i++)
  {
    int i1 = (i+1)%n;
    glColor3d(0.3,0.8,0.35);
    drawSegment(pts[i],pts[i1]);
  }
  for(int i=0;i<n;i++)
  {
    glColor3d(0.8,0.1,0.35);
    drawPoint(pts[i]);
  }
}

void PDiskPage::hVertex(const GmLib::Complex &c)
{
  glVertex(diskToWorld(m_transformStack.back()*c));
}

void PDiskPage::hVertex(const GmLib::Complex &c, double z)
{
  QPointF p = diskToWorld(m_transformStack.back()*c);
  glVertex3d(p.x(),p.y(),z);
}


void PDiskPage::drawPattern1()
{
  int n = 3, m = 7;
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
    //HTransform tr2 = m_transformStack.back();

    for(int j=0;j<n;j++)
    {
      int mm = 7;
      for(int k=1;k<mm;k++)
      {
        double s = (double)k/mm;
        Complex b = m_tess->border(j,s);
        double r = b.getNorm();
        double h = 0.0001;
        Complex rot = Complex::rot(pi*30/180); // pi*(120*(1-s)+180*s)/180);
        Complex db = rot * (m_tess->border(j,s+h)-m_tess->border(j,s-h));
        Complex b1 = b + db.normalized()*(r*0.45);

        glBegin(GL_LINE_STRIP);
        glColor3d(0,0,0);
        glLineWidth(2.0);
        int qm = 50;
        for(int q=0;q<qm;q++)
        {
          double t = (double)q/(qm-1);
          Complex p = (1-t)*(1-t)*Complex(0,0)+t*t*b+2*t*(1-t)*b1;
          double rd = (m_transformStack.back()*p).getNorm();
          //glColor3d(r*0.5,r*0.8,1);
          glColor3d(rd*0.5,rd*0.5,rd*0.5);
          hVertex(p);
        }
        glEnd();
      }
    }
    popTransform();
  }
  glLineWidth(1.0);

}

void PDiskPage::drawPattern2()
{
  glClear(GL_DEPTH_BUFFER_BIT);
  glEnable(GL_DEPTH_TEST);
    int n = 5, m = 5;
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
      for(int j=0;j<n;j++)
      {
        double r0 = 0.42;
        double r1 = 0.53;
        double ff = 0.8;

        double rr[] = {r0,ff*r0+(1-ff)*r1,(1-ff)*r0+ff*r1,r1};
        double col0[3] = {0.0,0.0,0.0};
        double col1[3] = {0.1,0.6,0.95};
        double *colors[] = {col0,col1,col1,col0};

        int nn = 50;
        for(int h=0;h<3;h++)
        {
          double ra = rr[h], rb = rr[h+1];

          glBegin(GL_QUAD_STRIP);
          for(int k=0;k<nn;k++)
          {
            double t = (double)k/(double)(nn-1);
            double s = 0.8 + t;
            double phi = (s+j)*2*pi/n;
            Complex c = Complex::rot(phi);
            // glColor3d(t,0.0,t);
            double z = 0.01*t;
            glColor3dv(colors[h]);
            hVertex(ra*c,z);
            glColor3dv(colors[h+1]);
            hVertex(rb*c,z);
          }
          glEnd();
        }
      }
      popTransform();
    }
  glDisable(GL_DEPTH_TEST);

}


void PDiskPage::drawPattern3()
{
  glClear(GL_DEPTH_BUFFER_BIT);
  glEnable(GL_DEPTH_TEST);
    int n = 6, m = 4;
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
      for(int j=0;j<n;j++)
      {
        glColor3d(0,0,0);
        int n0 = 10, n1 = 10;
        for(int k0=0;k0<n0;k0++)
        {
          double t0a = (double)k0/n0;
          double t0b = (double)(k0+1)/n0;
          Complex p0 = m_tess->border(j,0.5*t0a);
          Complex p1 = m_tess->border(j,0.5*t0b);
          glBegin(GL_QUAD_STRIP);
          for(int k1=0;k1<n1;k1++)
          {
            double t1 = (double)k1/(n1-1);
            t1 = 0.001+t1*0.999;
            hVertex(t1*p0);
            hVertex(t1*p1);
          }
          glEnd();
        }
      }
      popTransform();
    }
  glDisable(GL_DEPTH_TEST);

}

void PDiskPage::drawPattern4()
{
  int n0 = 60, n1 = 60;
  std::vector<Complex> pts(n0*n1);
  for(int i=0;i<n0;i++)
  {
    double im = 1 + exp(i*0.1);
    for(int j=0;j<n1;j++)
    {
      double re = -pi + 2*pi*j/(n1-1);
      Complex z(re,im);

      pts[i*n1+j] = (Complex(0,1)*z+1)/(z+Complex(0,1));
    }
  }

  glEnable(GL_BLEND);
  glBlendFunc(GL_SRC_ALPHA,GL_ONE_MINUS_SRC_ALPHA);
  glColor4d(0.8,0.9,0.95,0.4);
  for(int i=0;i+1<n0;i++)
  {
    glBegin(GL_QUAD_STRIP);
    for(int j=0;j<n1;j++)
    {
      hVertex(pts[i*n1+j]);
      hVertex(pts[(i+1)*n1+j]);
    }
    glEnd();
  }
  glDisable(GL_BLEND);
  glColor3d(0.0,0.0,0.0);
  glBegin(GL_LINE_STRIP);
  for(int i=n0-1;i>=0;i--) hVertex(pts[i*n1+0]);
  for(int j=0;j<n1;j++) hVertex(pts[j]);
  for(int i=0;i<n0;i++) hVertex(pts[i*n1+n1-1]);
  glEnd();

}

struct StrokePoint {QPointF p; double thick; double col;};

void PDiskPage::drawStroke(std::vector<GmLib::Complex> &stroke)
{
  int m = (int)stroke.size();
  if(m<3) return;
  std::vector<StrokePoint> pts(m);
  for(int i=0;i<m;i++)
  {
    Complex c = m_transformStack.back()*stroke[i];
    pts[i].thick = 0.4+3*getThickness(c);
    if(i<4) pts[i].thick *= (double)i/3;
    else if(i>m-1-4) pts[i].thick *= (double)(m-1-i)/3;
    pts[i].col = c.getNorm()*0.5;
    pts[i].p = diskToWorld(c);
  }

  glBegin(GL_TRIANGLE_STRIP);
  for(int i=0;i<m;i++)
  {
    QPointF dir;
    if(i==0) dir=pts[1].p-pts[0].p;
    else if(i==m-1) dir=pts[m-1].p-pts[m-2].p;
    else dir=pts[i+1].p-pts[i-1].p;
    double dirLength = sqrt(dir.x()*dir.x()+dir.y()*dir.y());
    if(dirLength>0.0)
      dir = dir * (pts[i].thick/dirLength);
    QPointF d(-dir.y(), dir.x());
    QPointF p = pts[i].p;
    glColor3d(pts[i].col,pts[i].col,pts[i].col);
    glVertex(p+d);
    glVertex(p-d);
  }
  glEnd();
}

void PDiskPage::drawStrokes()
{
  for(int i=0;i<(int)m_strokes.size();i++)
    drawStroke(m_strokes[i]);
}



void PDiskPage::paintGL()
{
  glClear(GL_COLOR_BUFFER_BIT);
  // glEnable(GL_MULTISAMPLE);

  double r = m_scale;
  const QPointF center = m_pan;

  // disk background
  glColor3d(1,1,1);
  fillCircle(center,r,250);

  if(m_oldStatus != m_status)
  {
    m_oldStatus = m_status;
    m_transformStack.clear();
    m_transformStack.push_back(HTransform());
    if(m_status == 1)
    {
      m_strokes.clear();
    }
  }

  if(m_status == 0)
  {
    // punti e rette
    glColor3d(0.3,0.8,0.35);
    drawLine(m_pts[0],m_pts[1], Color(0.3,0.8,0.35));
    glColor3d(0.3,0.8,0.8);
    drawLine(m_pts[2],m_pts[3], Color(0.3,0.8,0.8));
    // drawLine(m_pts[0],m_pts[3]);
    drawLine(m_pts[4], m_pts[3], Color(0.3,0.8,0.8)); // Complex(0.3,-0.2)

    for(int i=0;i<(int)m_pts.size();i++)
    {
      if(i==m_currentPoint)
        glColor3d(0.8,0.8,0.9);
      else
        glColor3d(0.8,0.2,0.35);
      drawPoint(m_pts[i]);
    }
  }
  else if(m_status == 1)
  {
    // figure simili
      if(m_repeatDrawings)
      {
        HTransform tr0 = HTransform::translation(Complex(-0.3,-0.2));
        HTransform tr1 = HTransform::translation(Complex(0.5,0.1)); // 2->5
        HTransform tr = tr0;
        for (int i=0;i<30;i++)
        {
          pushTransform();
          transform(tr);
          tr = tr * tr1;
          drawStrokes();
          popTransform();
         }
      }
      else
      {
        drawStrokes();
      }

/*

      else
    {
      HTransform tr0 = HTransform::translation(Complex(-0.3,-0.2));
      HTransform tr1 = HTransform::translation(Complex(0.2,0.1));
      HTransform tr = tr0;
      for (int i=0;i<30;i++)
      {
        pushTransform();
        transform(tr);
        tr = tr * tr1;
        drawPolygon(m_pts[0], 3);
        popTransform();
       }
     }
     */
  }
  else if(m_status == 2)
  {
    // ottagono
    drawPolygon(m_pts[0], 8);
  }
  else if(m_status == 3)
  {
    // ottagono con due vicini
    int n = 8; // , m = 3;
    double phi = 2*pi/n;
    Complex c = m_pts[0];
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
  }
  else if(4<=m_status && m_status<=6 || m_status==10)
  {
    // tre pattern diversi
    int nn[][2] = {{8,3},{6,4},{5,5}};
    int k = m_status-4;
    if(m_status>6) k = 0;
    int n = nn[k][0], m = nn[k][1];
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

    // disegna l'area coperta dalla pseudosfera
    if(m_status>6) drawPattern4();

  }
  else if(m_status == 7)
  {
    // ghirigori
    /*
    glColor3d(0.3,0.8,0.35);
    drawLine(Complex(-0.1,-0.4),Complex(0.1,-0.4), Color(0.3,0.8,0.35));

    Complex p(0,0.3);

    int n = 5;
    for(int i=0;i<n;i++)
    {
      double t = (double)i/(double)(n-1);
      glColor3d(0.2,0.8,0.8);
      double phi = 1.0 * 2*(t-0.5);
      drawLine(p,p + 0.01*Complex(cos(phi),sin(phi)), Color(0.3,0.8,0.35));
    }
    glColor3d(0.8,0.2,0.01);
    drawPoint(p, 2);
    */
    drawPattern1();
  }
  else if(m_status == 8)
  {
    // anelli
/*
    glColor3d(0.1,0.6,0.25);
    drawPolygon(Complex(0.1,0.0), 3);
    drawPolygon(Complex(0.3,0.0), 3);
    drawPolygon(Complex(0.5,0.0), 3);
    drawPolygon(Complex(0.7,0.0), 3);
    drawPolygon(Complex(0.9,0.0), 3);
    */
    drawPattern2();
    //    drawPattern3();
  }
  else if(m_status == 9)
  {
    drawPattern3();
  }

  // contorno del disco
  glLineWidth(4);
  glColor3d(0,0,0);
  drawCircle(center,r,200);
  glLineWidth(1);

}

void PDiskPage::mousePressEvent(QMouseEvent *e)
{
  m_panning = false;
  m_drawing = false;
  m_currentPoint = -1;
  m_lastPos = e->pos();
  // grabMouse();

  if(m_status==1 && e->button() == Qt::LeftButton)
  {
    m_drawing = true;
    m_strokes.push_back(std::vector<Complex>());
    Complex c = worldToDisk(winToWorld(e->pos()));
    m_strokes.back().push_back(m_transformStack.back().inverse() * c);
    return;
  }

  if(e->button() != Qt::LeftButton)
  {
    m_panning = true;
    updateGL();

    return;
  }

  QPointF pos = winToWorld(e->pos());
  int k = -1;
  if(m_status == 0)
  {
    double minD2 = 0;
    for(int i=0;i<(int)m_pts.size();i++)
    {
      Complex c = m_transformStack.back() * m_pts[i];
      QPointF ipos = diskToWorld(c);
      double d2 = getNorm2(pos-ipos);
      if(k<0 || d2<minD2) {minD2=d2; k=i;}
    }
    if(minD2<100)
      m_currentPoint = k;
  }
  else
    m_currentPoint = 0;
  // grabMouse();
  updateGL();
}

void PDiskPage::mouseMoveEvent(QMouseEvent *e)
{
  if(m_drawing)
  {
    Complex c = worldToDisk(winToWorld(e->pos()));
    m_strokes.back().push_back(m_transformStack.back().inverse() * c);
    updateGL();
    return;
  }
  if(m_panning)
  {
    QPoint delta = e->pos() - m_lastPos;
    m_lastPos = e->pos();
    Complex c(delta.x()*0.01,-delta.y()*0.01);
    m_transformStack.back() = HTransform::translation(c) * m_transformStack.back();
  }
  else if(m_currentPoint>=0)
  {
    Complex c = m_transformStack.back().inverse() * worldToDisk(winToWorld(e->pos()));
    m_pts[m_currentPoint] = boundPoint(c);
  }
  updateGL();
}

void PDiskPage::mouseReleaseEvent(QMouseEvent *)
{
  m_currentPoint = -1;
  // releaseMouse();
}

  #define GL_MULTISAMPLE 0x809D

void PDiskPage::saveImage()
{
  QGLFormat fmt;
  fmt.setDoubleBuffer(false);
  fmt.setAlpha(false);
  fmt.setSampleBuffers(true);
  QGLPixelBuffer buffer(4000,4000,fmt);
  buffer.makeCurrent();
  glEnable(GL_MULTISAMPLE);
  //glEnable(GL_LINE_SMOOTH);
  //glHint(GL_LINE_SMOOTH_HINT, GL_NICEST);
  //glEnable(GL_BLEND);
  //glBlendFunc(GL_SRC_ALPHA,GL_ONE_MINUS_SRC_ALPHA);
  glViewport(0,0,buffer.width(),buffer.height());
  glMatrixMode(GL_PROJECTION);
  glLoadIdentity();
  double r = m_scale+8;
  const QPointF center = m_pan;
  glOrtho(center.x()-r,center.x()+r,center.y()-r,center.y()+r,-1,1);
  glMatrixMode(GL_MODELVIEW);
  glClearColor(1,1,1,1);
  // glColorMask(true,true,true,false);
  paintGL();
  glFlush();
  glFinish();
  buffer.doneCurrent();
  makeCurrent();
  buffer.toImage().save("screenshot.png");
}

void PDiskPage::savePattern1()
{
  QImage image(2000,2000,QImage::Format_RGB32);
  image.fill(qRgb(255,255,255));
  QPainter pa(&image);
  pa.setRenderHints(QPainter::HighQualityAntialiasing|QPainter::Antialiasing);
  //pa.setPen(QPen(Qt::black, 5));

  int n = 3, m = 7;
  HTessellation tess(n,m);
  tess.init(5000);

  for(int i=0;i<tess.getFaceCount();i++)
  {
    HTransform tr = tess.getFaceTransform(i);
    pushTransform();
    transform(tr);
    for(int j=0;j<n;j++)
    {
      int mm = 7;
      for(int k=1;k<mm;k++)
      {
        double s = (double)k/mm;
        Complex b = tess.border(j,s);
        double r = b.getNorm();
        double h = 0.0001;
        Complex rot = Complex::rot(pi*30/180); // pi*(120*(1-s)+180*s)/180);
        Complex db = rot * (tess.border(j,s+h)-tess.border(j,s-h));
        Complex b1 = b + db.normalized()*(r*0.45);

        QPainterPath pp;

        int qm = 50;
        for(int q=0;q<qm;q++)
        {
          double t = (double)q/(qm-1);
          Complex p = (1-t)*(1-t)*Complex(0,0)+t*t*b+2*t*(1-t)*b1;
          // double r = (m_transformStack.back()*p).getNorm();
          Complex c = m_transformStack.back()*p;
          QPointF pt(1000.0+950*c.re,1000.0+950*c.im);
          // glColor3d(r*0.5,r*0.5,r*0.5);
          if(q==0) pp.moveTo(pt);
          else pp.lineTo(pt);
        }
        int grey = (int)(255.0*0.8*r);
        pa.setPen(QColor(grey,grey,grey));
        pa.drawPath(pp);
      }
    }
    popTransform();
  }
  pa.setPen(QPen(Qt::black, 6));
  pa.drawEllipse(50,50,1900,1900);
  image.save("snapshot.png");
  qApp->quit();
}

// Per la studentessa: "reticolo iperbolico"
void PDiskPage::savePattern2()
{
  int halfsize=2000;
  int halfsize2 = (int)(halfsize * 0.99);
  QImage image(halfsize*2,halfsize*2,QImage::Format_RGB32);
  image.fill(qRgb(255,255,255));
  QPainter pa(&image);
  pa.setRenderHints(QPainter::HighQualityAntialiasing|QPainter::Antialiasing);
  //pa.setPen(QPen(Qt::black, 5));

#ifdef GRID1

  int n = 8, m=3;
  HTessellation tess(n,m);
  tess.init(5000);

  for(int i=0;i<tess.getFaceCount();i++)
  {
    HTransform tr = tess.getFaceTransform(i);
    pushTransform();
    transform(tr);
    for(int j=0;j<n;j++)
    {
      {
        QPainterPath pp;
        int nk = 10;
        for(int k=0;k<nk;k++)
        {
          double t = (double)k/(double)(nk-1);
          Complex c = m_transformStack.back()*(tess.border(j,0.5)*t);
          QPointF pt(halfsize+halfsize2*c.re,halfsize+halfsize2*c.im);
          if(k==0) pp.moveTo(pt);
          else pp.lineTo(pt);
        }
        int grey = 80;
        pa.setPen(QPen(QColor(grey,grey,grey),3));
        pa.drawPath(pp);
      }
      if(false)
      {
        QPainterPath pp;
        int nk = 10;
        for(int k=0;k<nk;k++)
        {
          double t = (double)k/(double)(nk-1);
          Complex c = m_transformStack.back()*(tess.border(j,t));
          QPointF pt(halfsize+halfsize2*c.re,halfsize+halfsize2*c.im);
          if(k==0) pp.moveTo(pt);
          else pp.lineTo(pt);
        }
        int grey = 180;
        pa.setPen(QPen(QColor(grey,grey,grey),3));
        pa.drawPath(pp);
      }
    }
    popTransform();
  }

/*
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
    */

  /*
  for(int i=0;i<tess.getFaceCount();i++)
  {
    HTransform tr = tess.getFaceTransform(i);
    pushTransform();
    transform(tr);
    for(int j=0;j<n;j++)
    {
      int mm = 7;
      for(int k=1;k<mm;k++)
      {
        double s = (double)k/mm;
        Complex b = tess.border(j,s);
        double r = b.getNorm();
        double h = 0.0001;
        Complex rot = Complex::rot(pi*30/180); // pi*(120*(1-s)+180*s)/180);
        Complex db = rot * (tess.border(j,s+h)-tess.border(j,s-h));
        Complex b1 = b + db.normalized()*(r*0.45);

        QPainterPath pp;

        int qm = 50;
        for(int q=0;q<qm;q++)
        {
          double t = (double)q/(qm-1);
          Complex p = (1-t)*(1-t)*Complex(0,0)+t*t*b+2*t*(1-t)*b1;
          // double r = (m_transformStack.back()*p).getNorm();
          Complex c = m_transformStack.back()*p;
          QPointF pt(1000.0+950*c.re,1000.0+950*c.im);
          // glColor3d(r*0.5,r*0.5,r*0.5);
          if(q==0) pp.moveTo(pt);
          else pp.lineTo(pt);
        }
        int grey = (int)(255.0*0.8*r);
        pa.setPen(QColor(grey,grey,grey));
        pa.drawPath(pp);
      }
    }
    popTransform();
  }
  */

#else
  pa.setPen(QPen(Qt::black,3));
  pa.drawLine(halfsize,halfsize-halfsize2,halfsize,halfsize+halfsize2);
  pa.drawLine(halfsize-halfsize2,halfsize,halfsize+halfsize2,halfsize);
  HTransform tr;
  for(int i=0;i<300;i++)
  {
    tr = tr * HTransform::translation(Complex(0.05,0));
    for(int j=0;j<4;j++)
    {
      QPainterPath pp;
      int nk = 500;
      for(int k=0;k<nk;k++)
      {
        double t = (double)k/(double)(nk-1);
        Complex c = tr * Complex(0,(-1+2*t)*0.9999);
        c = Complex::rot(3.14152*0.5*j) * c;
        QPointF pt(halfsize+halfsize2*c.re,halfsize+halfsize2*c.im);
        if(k==0) pp.moveTo(pt);
        else pp.lineTo(pt);
      }
      pa.drawPath(pp);
    }
  }


#endif

  pa.setPen(QPen(Qt::black, 6));
  pa.drawEllipse(halfsize*QPointF(1,1), halfsize2, halfsize2);
  image.save("snapshot.png");
  qApp->quit();
}



void PDiskPage::keyPressEvent(QKeyEvent *e)
{
  if(Qt::Key_0 <= e->key() && e->key() <= Qt::Key_9)
  {
    m_status = e->key() - Qt::Key_0;
  }
  else if(e->key() == Qt::Key_Left)
  {
    if(m_status>0) {m_status--; }
  }
  else if(e->key() == Qt::Key_Right)
  {
    m_status++;
  }
  else if(e->key() == Qt::Key_N)
  {
    m_transformStack.back() = HTransform();
    m_status++;
  }
  else if(e->key() == Qt::Key_S)
  {
    savePattern2();
  }
  else if(e->key() == Qt::Key_R)
  {
    m_repeatDrawings = !m_repeatDrawings;
  }
  else
  {
    e->ignore();
    return;
  }
  updateGL();
}

void PDiskPage::wheelEvent(QWheelEvent*)
{
}



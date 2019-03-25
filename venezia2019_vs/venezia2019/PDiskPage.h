#ifndef PDISKPAGE_H
#define PDISKPAGE_H

#include "OpenGLPage.h"
#include "Page.h"
#include <QPointF>
#include "HTransform.h"
#include "HTessellation.h"
#include "Complex.h"
#include <vector>
#include <set>

#include <QGLWidget>
#include <QGLContext>
#include <set>
#include <QPointF>
#include <qglbuffer.h>


struct Color;

class HLine {
  GmLib::Complex m_pa, m_pb;
  GmLib::Complex m_center;
  double m_radius;

public:
  HLine() : m_pa(-1,0), m_pb(1,0), m_center(0,0), m_radius(0) {}

  GmLib::Complex getPoint(double t) const;
  GmLib::Complex getDir(double t) const;

  static HLine makeSegment(const GmLib::Complex &pa, const GmLib::Complex &pb);
  static HLine makeLine(const GmLib::Complex &pa, const GmLib::Complex &pb);
};


class PDiskPage : public OpenGLPage
{
protected:
  int m_status;
  int m_oldStatus;

private:
  double m_scale;
  QPointF m_pan;
  std::vector<GmLib::HTransform> m_transformStack;

  std::vector<GmLib::Complex> m_pts;
  std::vector<std::vector<GmLib::Complex> > m_strokes;

  int m_currentPoint;
  QPoint m_lastPos;
  bool m_panning;
  bool m_drawing;
  bool m_repeatDrawings;
  GmLib::HTessellation *m_tess;

public:
  PDiskPage();

  QPointF diskToWorld(const GmLib::Complex &c) const {
    return QPointF(c.re,c.im) * m_scale + m_pan;
  }
  GmLib::Complex worldToDisk(const QPointF &p) const {
    QPointF p1 = (p-m_pan) * (1.0/m_scale);
    return GmLib::Complex(p1.x(),p1.y());
  }

  QPoint worldToWin(const QPointF &p) const {
    return QPoint((int)(p.x()+0.5), (int)(height()-p.y()+0.5));
  }

  QPointF winToWorld(const QPoint &p) const {
    return QPointF(p.x(), height()-p.y());
  }

  double getThickness(const GmLib::Complex &c) const;
  GmLib::Complex boundPoint(const GmLib::Complex &c) const;
  void drawPoint(const GmLib::Complex &c, double rFactor=1.0) const;
  void drawHLine(const HLine &hline, const Color &color) const;
  void drawSegment(const GmLib::Complex &c0, const GmLib::Complex &c1) const;
  void drawLine(const GmLib::Complex &c0, const GmLib::Complex &c1, const Color &color) const;

  void drawPolygon(const GmLib::Complex &c0, int n) const;

  void pushTransform() {m_transformStack.push_back(m_transformStack.back());}
  void popTransform() {m_transformStack.pop_back();}
  void transform(const GmLib::HTransform &transform) { m_transformStack.back() = m_transformStack.back() * transform;}

  void hVertex(const GmLib::Complex &c);
  void hVertex(const GmLib::Complex &c, double z);

  void drawPattern1();
  void drawPattern2();
  void drawPattern3();
  void drawPattern4();

  void drawStroke(std::vector<GmLib::Complex> &stroke);
  void drawStrokes();

  void saveImage();
  void savePattern1();
  void savePattern2();

  void start();

  void initializeGL();
  void resizeGL(int width, int height);
  void paintGL();

protected:

  void mousePressEvent(QMouseEvent *e);
  void mouseMoveEvent(QMouseEvent *e);
  void mouseReleaseEvent(QMouseEvent *e);

  void showEvent(QShowEvent *);

  void keyPressEvent(QKeyEvent *e);
  void wheelEvent(QWheelEvent*);

};


class PDiskPage2 : public PDiskPage
{
public:
  PDiskPage2() {
    m_status = 10;
  }
};


#endif // PDISKPAGE_H

#ifndef GUTIL_H
#define GUTIL_H

#include <QPointF>
#include <math.h>
#include <QGLWidget>
#include <QVector3D>
#include "Point3.h"
#include <gl/GLU.h>


inline double getNorm2(const QPointF &p) {return p.x()*p.x()+p.y()*p.y();}
inline double getNorm(const QPointF &p) {return sqrt(getNorm2(p)); }

inline QPointF normalize(const QPointF &p) {
  return p*(1.0/getNorm(p));
}
inline float dot(const QPointF &a, const QPointF &b) {
  return a.x()*b.x()+a.y()*b.y();
}

inline QPointF rotate90(const QPointF &p) {
  return QPointF(-p.y(),p.x());
}

inline void glVertex(const QPointF &p) {glVertex2f(p.x(),p.y());}
inline void glVertex(const GmLib::Point3 &p) {glVertex3d(p.x(),p.y(),p.z());}
inline void glNormal(const GmLib::Point3 &p) {glNormal3d(p.x(),p.y(),p.z());}
inline void glTexCoord(const QPointF &uv) {glTexCoord2f(uv.x(),1-uv.y());}
inline void glTranslate(const GmLib::Point3 &p) {glTranslated(p.x(),p.y(),p.z());}

void drawCircle(const QPointF &center, double r, int m=20);
inline void drawCircle(float x, float y, double r, int m=20) {drawCircle(QPointF(x,y), r,m);}
void fillCircle(const QPointF &center, double r, int m=20);
inline void fillCircle(float x, float y, double r, int m=20) {fillCircle(QPointF(x,y), r,m);}

#define arrayCount(A) ((int)(sizeof(A)/sizeof(A[0])))

inline double clamp(double v, double min, double max) {return qMax(min,qMin(max,v));}
inline double clamp(double v) {return clamp(v,0,1);}
inline double smooth(double v) {return v*v*(3-2*v);}

void drawSphere(const GmLib::Point3 &center, double radius, int n = 20, int m = 20);
void drawCylinder(const GmLib::Point3 &c0, const GmLib::Point3 &c1, double radius, int n = 20, int m = 5);
void drawCone(const GmLib::Point3 &c0, double r0, const GmLib::Point3 &c1, double r1, int n = 20, int m = 5);
void drawTriangle(const GmLib::Point3 &p0, const GmLib::Point3 &p1, const GmLib::Point3 &p2);


struct Color {
  double r,g,b,m;
  Color() : r(0), g(0), b(0), m(0) {}
  Color(double _r, double _g, double _b, double _m=1.0) : r(_r), g(_g), b(_b), m(_m) {}
};

inline Color mix(const Color &c0, const Color &c1, double t)
{
  return Color((1-t)*c0.r+t*c1.r,(1-t)*c0.g+t*c1.g,(1-t)*c0.b+t*c1.b,(1-t)*c0.m+t*c1.m);
}


void setColor(double r, double g, double b, double m=1.0);
inline  void setColor(const Color &c) {setColor(c.r,c.g,c.b,c.m);}

#endif // GUTIL_H

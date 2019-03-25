#ifndef POINT3_H
#define POINT3_H

#include <qvector3d.h>

namespace GmLib {

  class Point3 {
    double m_coords[3];
  public:
    Point3() {m_coords[0]=m_coords[1]=m_coords[2]=0;}
    Point3(const QVector3D &pos) { m_coords[0]=pos.x(); m_coords[1]=pos.y(); m_coords[2]=pos.z(); }
    Point3(double x, double y, double z) {m_coords[0]=x;m_coords[1]=y;m_coords[2]=z;}
    Point3(const double *coords) {m_coords[0]=coords[0];m_coords[1]=coords[1];m_coords[2]=coords[2];}
    double x() const {return m_coords[0];}
    double y() const {return m_coords[1];}
    double z() const {return m_coords[2];}
    Point3 &operator+=(const Point3 &p) {
      m_coords[0]+=p.m_coords[0];m_coords[1]+=p.m_coords[1];m_coords[2]+=p.m_coords[2];
      return *this;
    }
    Point3 &operator-=(const Point3 &p) {
      m_coords[0]-=p.m_coords[0];m_coords[1]-=p.m_coords[1];m_coords[2]-=p.m_coords[2];
      return *this;
    }
    Point3 &operator*=(double k) {
      m_coords[0]*=k;m_coords[1]*=k;m_coords[2]*=k;
      return *this;
    }
    Point3 operator+(const Point3 &p)const {Point3 ret(*this); return ret+=p;}
    Point3 operator-(const Point3 &p)const {Point3 ret(*this); return ret-=p;}
    Point3 operator-()const {return Point3(-m_coords[0],-m_coords[1],-m_coords[2]);}
    Point3 operator*(double k) const {Point3 ret(*this); return ret*=k;}
    double getNorm2() const {return m_coords[0]*m_coords[0]+m_coords[1]*m_coords[1]+m_coords[2]*m_coords[2];}
    double getNorm() const;
    Point3 normalized() const { return *this * (1.0/getNorm());}
    const double *coords() const {return m_coords;}
  };

  inline Point3 operator*(double k, const Point3 &p) {return p*k;}

  inline double dot(const Point3&a, const Point3&b)
  {
    return a.x()*b.x()+a.y()*b.y()+a.z()*b.z();
  }
  inline Point3 cross(const Point3&a, const Point3&b)
  {
    return Point3(
      a.y()*b.z()-a.z()*b.y(),
      a.z()*b.x()-a.x()*b.z(),
      a.x()*b.y()-a.y()*b.x());
  }

} // namespace GmLib

#endif // POINT3_H

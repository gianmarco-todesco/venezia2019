#ifndef HTRANSFORM_H
#define HTRANSFORM_H

#include "Complex.h"

namespace GmLib {

class HTransform
{
public:
  Complex m_a, m_b, m_c, m_d;


  HTransform(): m_a(1), m_b(0), m_c(0), m_d(1) {}
  HTransform(const Complex &a, const Complex &b, const Complex &c) : m_a(a), m_b(b), m_c(c), m_d(1) {}

  static HTransform translation(const Complex &c);
  static HTransform rotation(double rad);

  Complex operator*(const Complex &z) const {
    return (m_a*z+m_b)/(m_c*z+m_d);
  }

  HTransform operator*(const HTransform &transf) const {
    HTransform ret;
    ret.m_a = m_a * transf.m_a + m_b * transf.m_c;
    ret.m_b = m_a * transf.m_b + m_b * transf.m_d;
    ret.m_c = m_c * transf.m_a + m_d * transf.m_c;
    ret.m_d = m_c * transf.m_b + m_d * transf.m_d;
    Complex id = Complex(1) / ret.m_d;
    ret.m_a = ret.m_a * id;
    ret.m_b = ret.m_b * id;
    ret.m_c = ret.m_c * id;
    ret.m_d = Complex(1);
    return ret;
  }

  HTransform inverse() const {
    return HTransform(m_d/m_a,-m_b/m_a,-m_c/m_a);
  }



};

} // namespace GmLib

#endif // HTRANSFORM_H

#ifndef COMPLEX_H
#define COMPLEX_H

#include <cmath>

namespace GmLib {

class Complex
{
public:
  double re, im;
  Complex(double re=0.0, double im=0.0) : re(re), im(im) {}

  Complex conj() const {return Complex(re,-im);}
  Complex operator+(const Complex &c) const { return Complex(re+c.re,im+c.im); }
  Complex operator-(const Complex &c) const { return Complex(re-c.re,im-c.im); }
  Complex operator-() const { return Complex(-re,-im); }
  Complex operator*(double k) const {return Complex(re*k,im*k); }
  Complex operator*(const Complex &c) const {return Complex(re*c.re-im*c.im,re*c.im+im*c.re); }
  Complex operator/(const Complex &c) const {
    double d = 1.0/(c.re*c.re+c.im*c.im);
    return Complex(d*(re*c.re+im*c.im),d*(im*c.re-re*c.im));
  }

  const Complex &operator*=(const Complex &c) {return *this = *this * c; }
  const Complex &operator*=(double k) {return *this = *this * k; }


  double getNorm2() const {return re*re+im*im;}
  double getNorm() const {return sqrt(getNorm2());}
  Complex normalized() const {return *this * (1.0/getNorm()); }

  static Complex rot(double phi) {return Complex(std::cos(phi), std::sin(phi)); }
};

inline Complex operator*(double k, const Complex &c) {return c*k;}
inline double dot(const Complex &a, const Complex &b) { return a.re*b.re+a.im*b.im;}


} // namespace GmLib

#endif // COMPLEX_H

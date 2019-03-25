#include "HTransform.h"

namespace GmLib {

HTransform HTransform::translation(const Complex &c)
{
  return HTransform(Complex(1), Complex(c), Complex(c.conj()));
}

HTransform HTransform::rotation(double rad)
{
  return HTransform(Complex(cos(rad), sin(rad)), Complex(0), Complex(0));
}

}

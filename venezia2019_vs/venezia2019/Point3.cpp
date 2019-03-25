#include "Point3.h"
#include <math.h>

using namespace GmLib;

double Point3::getNorm() const
{
  return sqrt(getNorm2());
}

#include "CircleLimit3Tessellation.h"

using namespace GmLib;

CircleLimit3Tessellation::CircleLimit3Tessellation()
: HTessellation(8,3)
{
}

Net::Face *CircleLimit3Tessellation::addFirstFace()
{
  Face *face = my(HTessellation::addFirstFace());
  face->c0 = 0;
  face->c1 = 1;
  face->c2 = 2;
  return face;
}

Net::Face *CircleLimit3Tessellation::addFace(HTessellation::HEdge *boundaryHEdge)
{
  Face *face = my(HTessellation::addFace(boundaryHEdge));
  Face *parentFace = my(boundaryHEdge->getRightFace());
  int c0 = parentFace->c0, c1 = parentFace->c1, c2 = parentFace->c2;
  int c3 = 6-(c0+c1+c2);
  int indexInFace = static_cast<HTessellation::HEdge*>(boundaryHEdge->getTween())->indexInFace;

  switch(indexInFace&0x3)
  {
    case 0:face->c0 = c0;  face->c1 = c2; face->c2 = c3; break;
    case 1:face->c0 = c0;  face->c1 = c3; face->c2 = c1; break;
    case 2:face->c0 = c1;  face->c1 = c3; face->c2 = c2; break;
    case 3:face->c0 = c1;  face->c1 = c2; face->c2 = c0; break;
  }

  return face;
}

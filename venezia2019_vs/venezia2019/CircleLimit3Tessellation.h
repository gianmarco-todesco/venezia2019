#ifndef CIRCLELIMIT3TESSELLATION_H
#define CIRCLELIMIT3TESSELLATION_H

#include "HTessellation.h"


class CircleLimit3Tessellation : public GmLib::HTessellation {
public:
  class Face : public HTessellation::Face {
  public:
    int c0, c1, c2;
    Face() : HTessellation::Face(), c0(-1), c1(-1), c2(-1) {}
  };

  CircleLimit3Tessellation();

  Net::Face   *createFace()   { return add(new Face()); }

  inline Face *my(Net::Face *f) const { return static_cast<Face*>(f); }
  inline HEdge *my(Net::HEdge *he) const { return HTessellation::my(he); }


protected:

  Net::Face *addFirstFace();
  Net::Face *addFace(HEdge *boundaryHEdge);
};

#endif // CIRCLELIMIT3TESSELLATION_H

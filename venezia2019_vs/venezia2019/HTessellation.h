#ifndef HTESSELLATION_H
#define HTESSELLATION_H

#include "Net.h"
#include "HTransform.h"
#include <vector>
#include <set>

namespace GmLib {

class HTessellation : public Net
{
public:

  class HEdge : public Net::HEdge {
  public:
    int indexInFace; 
    HEdge() : Net::HEdge(), indexInFace(0) {} 
  };
  class Face : public Net::Face { 
  public:
    HTransform transform;
    Face() : Net::Face(), transform() {};
  };

  HTessellation(int n, int m);

  Net::HEdge  *createHEdge()  { return add(new HEdge()); }
  Net::Face   *createFace()   { return add(new Face()); }

  Complex border(int edgeIndex, double t) const;

  //inline HEdge *getMyHEdge(int index) {
  //  return static_cast<HEdge*>(getHEdge(index));
  //}

  void init(int count);

  inline HEdge *my(Net::HEdge *he) const { return static_cast<HEdge*>(he); }
  inline Face *my(Net::Face *f) const { return static_cast<Face*>(f); }

  const HTransform &getFaceTransform(int index) const { return my(getFace(index))->transform; }

  int getN() const {return m_n;}
  int getM() const {return m_m;}

protected:

  virtual Net::Face *addFirstFace();
  virtual Net::Face *addFace(HTessellation::HEdge *boundaryHEdge);

private:

  int m_n, m_m;
  HTransform m_g0, m_g1;
  double m_r, m_d, m_r0, m_r1;
  Complex m_p0;
  std::set<int> m_boundary;


  double getHEdgePriority(Net::HEdge *he) const;
  void addToBoundary(Net::HEdge *he);
  void removeFromBoundary(Net::HEdge *he);
  void updateHEdges(Net::Face *face);
};


} // namespace GmLib

#endif // HTESSELLATION_H

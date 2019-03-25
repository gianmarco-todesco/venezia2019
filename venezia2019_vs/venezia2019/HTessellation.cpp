#include "HTessellation.h"
#include <cmath>
#include <map>
#include <assert.h>

using namespace std;
const double PI = 3.14159265;

using namespace GmLib;


HTessellation::HTessellation(int n, int m)
: m_n(n)
, m_m(m)
{
  double alpha = PI/n;
  double beta = PI/m + PI/2;
  double gamma = PI - alpha - beta;
  double snAlpha = sin(alpha);
  double snBeta = sin(beta);
  double snGamma = sin(gamma);
  assert(alpha+beta<PI);
  double q = 1.0/sqrt(snBeta*snBeta-snAlpha*snAlpha);
  m_r = snAlpha*q;
  m_d = snBeta*q;
  m_r0 = m_d-m_r;
  m_r1 = m_d * snGamma / snBeta;

  m_g0 = HTransform::rotation(2*PI/n);

  Complex v(m_r1*cos(alpha),m_r1*sin(alpha));
  m_p0 = v;
  m_g1 = HTransform::translation(v) * HTransform::rotation(2*PI/m) * HTransform::translation(-v);

}

void HTessellation::init(int count)
{
  assert(getFaceCount()==0);
  addFirstFace();
  map<int, double> priorityTable;
  for(int k=0;k<count;k++)
  {
    assert(!m_boundary.empty());
    int heIndex = -1;
    double bestPriority = 0;
    for(set<int>::iterator it = m_boundary.begin(); it != m_boundary.end(); ++it)
    {
      Net::HEdge *he = getHEdge(*it);
      double priority = 0;
      map<int, double>::iterator priorityIt = priorityTable.find(he->getIndex());
      if(priorityIt == priorityTable.end())
        priority = priorityTable[he->getIndex()] = getHEdgePriority(he);
      else
        priority = priorityIt->second;
      if(heIndex<0 || priority<bestPriority)
      {
        heIndex = he->getIndex();
        bestPriority = priority;
      }
    }
    addFace(my(getHEdge(heIndex)));

    for(int i=0;i<getHEdgeCount();i++)
    {
      Net::HEdge *he = getHEdge(i);
      bool b0 = he->getLeftFace()==0;
      bool b1 = m_boundary.count(i)>0;
      assert(b0==b1);
    }
  }
}



void HTessellation::updateHEdges(Net::Face *face)
{
  Net::HEdge *he = face->getHedge();
  int count = 0;
  int indexInFace = 0;
  do {
    my(he)->indexInFace = indexInFace++;
    he=he->getNext();
    assert(count<m_n);
    count++;
  } while(he != face->getHedge());
}


double HTessellation::getHEdgePriority(Net::HEdge *he) const
{
  assert(he->getLeftFace()==0);
  assert(he->getRightFace()!=0);
  Net::Face *f = he->getRightFace();
  const HTransform &transform = my(f)->transform;
  int indexInFace = my(he->getTween())->indexInFace;
  Complex c0 = transform * border(indexInFace,0);
  Complex c1 = transform * border(indexInFace,1);
  return (c0+c1).getNorm2();
}

void HTessellation::addToBoundary(Net::HEdge *he)
{
  assert(he->getLeftFace()==0);
  assert(he->getRightFace()!=0);
  m_boundary.insert(he->getIndex());
}

void HTessellation::removeFromBoundary(Net::HEdge *he)
{
  m_boundary.erase(he->getIndex());
}

Net::Face *HTessellation::addFirstFace()
{
  Net::Face *face = createFirstFace(m_n);
  updateHEdges(face);
  Net::HEdge *he = face->getHedge();
  do {
    addToBoundary(my(he->getTween()));
    he=he->getNext();
  } while(he != face->getHedge());

  return face;
  //my(face)->c0 = 0;
  //my(face)->c1 = 1;
  //my(face)->c2 = 2;
}


Net::Face *HTessellation::addFace(HEdge *boundaryHEdge)
{
  int edgesPerVertex = m_m;
  Net::HEdge *he2 = boundaryHEdge;
  Net::HEdge *he = he2->getTween();
  assert(he2->getLeftFace() == 0);
  assert(he->getLeftFace() != 0);
  Net::Face *parentFace = he->getLeftFace();


  Vertex *va = he2->getNextVertex();
  Vertex *vb = he->getNextVertex();

  removeFromBoundary(he2);
  int oldVertexCount = 2;
  Vertex *vaa = va;
  Net::HEdge *he3 = he2;
  int offset = 0;
  while(vaa->getEdgeCount()>=edgesPerVertex)
  {
    he3 = he3->getNext();
    removeFromBoundary(he3);
    vaa = he3->getNextVertex();
    oldVertexCount++;
    offset++;
  }
  Vertex *vbb = vb;
  he3 = he2->getPrev();
  assert(he3->getNextVertex() == vbb);
  while(vbb->getEdgeCount()>=edgesPerVertex)
  {
    removeFromBoundary(he3);
    he3 = he3->getPrev();
    vbb =he3->getNextVertex();
    oldVertexCount++;
  }

  // ii[] <- other vertices of face adjacent to ia->ib (i.e. ic, id....)
  std::vector<Vertex*> vv;
  Net::HEdge *he4 = he->getNext();
  for(;;)
  {
    Vertex *v = he4->getNextVertex();
    if(v==va) break;
    vv.push_back(v);
    he4 = he4->getNext();
  }
  int vertexCount = 2 + (int)vv.size();


  // add face
  Net::Face *face = growFace(vertexCount, vbb->getIndex(),vaa->getIndex());

  he4 = face->getHedge();
  int count=0;
  for(;;)
  {
    if(he4->getRightFace()==0) addToBoundary(he4->getTween());
    he4 = he4->getNext();
    if(count++>10) {assert(0); return 0;}
    if(he4 == face->getHedge()) break;
  }

  assert(he2->getLeftFace() == face);
  assert(he2->getRightFace() == parentFace);

  //updateFaceType(he2);

  HTransform transform = m_g1;
  int indexInFace = my(he2->getTween())->indexInFace;
  for(int i=0;i<indexInFace;i++)
    transform = m_g0 * transform;

  if(indexInFace&1)
  {
    transform = transform * m_g0;
    he2=he2->getNext();
  }
  face->setHedge(he2->getPrev());
  Face *faceData = my(face);
  Face *parentFaceData = my(parentFace);

  faceData->transform = parentFaceData->transform * transform;

  updateHEdges(face);
  return face;

}



Complex HTessellation::border(int edgeIndex, double t) const
{
  Complex p(m_p0.re-m_d, -m_p0.im * (1-2*t));
  p *= m_r/p.getNorm();
  p.re += m_d;
  double phi = 2*PI*edgeIndex/m_n;
  return Complex(cos(phi), sin(phi)) * p;
}



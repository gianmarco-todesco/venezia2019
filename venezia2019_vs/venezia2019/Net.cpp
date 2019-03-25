#include "Net.h"
#include <assert.h>



using namespace GmLib;


Net::Net()
{
}

//-----------------------------------------------------------------------------

Net::~Net()
{
  clear();
}

//-----------------------------------------------------------------------------

void Net::clear()
{
  for(int i=0;i<getVertexCount();i++) delete getVertex(i);
  for(int i=0;i<getEdgeCount();i++) delete getEdge(i);
  for(int i=0;i<getHEdgeCount();i++) delete getHEdge(i);
  for(int i=0;i<getFaceCount();i++) delete getFace(i);
  m_vertices.clear();
  m_edges.clear();
  m_hedges.clear();
  m_faces.clear();
}

//-----------------------------------------------------------------------------

void Net::createVertices(int count)
{
  if(count<=0) return;
  int index = (int)m_vertices.size();
  m_vertices.resize(index + count);
  for(int i=0;i<count;i++)
  {
    Vertex *v = m_vertices[index] = createVertex();
    v->m_index = index++;
  }
}

//-----------------------------------------------------------------------------

Net::Vertex *Net::add(Vertex*v)
{
  v->m_index  = getVertexCount();
  m_vertices.push_back(v);
  return v;
}

//-----------------------------------------------------------------------------

Net::Edge *Net::add(Edge*e)
{
  e->m_index  = getEdgeCount();
  m_edges.push_back(e);

  HEdge *he0 = createHEdge(),*he1 = createHEdge();
  he0->m_tween = he1;
  he1->m_tween = he0;
  e->m_hedge = he0;
  he0->m_edge = he1->m_edge = e;

  return e;
}

//-----------------------------------------------------------------------------

Net::HEdge *Net::add(HEdge*he)
{
  he->m_index = (int)m_hedges.size();
  m_hedges.push_back(he);
  return he;
}

//-----------------------------------------------------------------------------

Net::Face *Net::add(Face*f)
{
  f->m_index  = getFaceCount();
  m_faces.push_back(f);
  return f;
}

//-----------------------------------------------------------------------------


Net::Face *Net::createFace(const int ii[], int m)
{

  // create new face. set vertexCount
  Face *face = createFace();
  face->m_vertexCount = m;

  for(int i=0;i<m;i++)
  {
    Vertex *v0 = getVertex(ii[i]);
    Vertex *v1 = getVertex(ii[(i+1)%m]);
    HEdge *he0 = getHedge(v0,v1);
    if(!he0) he0 = getHedge(v0,(Face*)0);
    assert(he0!=0 || v0->getEdgeCount()==0);
    if(he0 && he0->getNextVertex() == v1)
    {
      assert(he0->getLeftFace() == 0);
      he0->m_leftFace = face;
      if(face->m_hedge == 0) face->m_hedge = he0;
    }
    else
    {
      v0->m_edgeCount++;
      v1->m_edgeCount++;
      HEdge *he1 = i==m-1 ? getHedge(v1,face) : getHedge(v1,(Face*)0);
      assert(he1!=0 || v1->getEdgeCount()==1);
      Edge *e = createEdge();
      HEdge *he = e->getHedge();
      he->m_nextVertex = v1;
      he->getTween()->m_nextVertex = v0;
      if(!he0) {v0->m_hedge = he; he->getTween()->linkTo(he); }
      else {he0->getPrev()->linkTo(he);he->getTween()->linkTo(he0);}
      if(!he1) {v1->m_hedge = he->getTween(); he->linkTo(he->getTween()); }
      else {he1->getPrev()->linkTo(he->getTween());he->linkTo(he1);}
      he->m_leftFace = face;
      if(face->m_hedge == 0) face->m_hedge = he;
      he->getTween()->m_leftFace = 0;
    }
  }
  return face;
}

//-----------------------------------------------------------------------------

Net::Face *Net::createFirstFace(int vertexCount)
{
  std::vector<Vertex*> vertices(vertexCount);
  std::vector<Edge*> edges(vertexCount);
  for(int i=0;i<vertexCount;i++)
  {
    vertices[i] = createVertex();
    edges[i] = createEdge();
  }

  Face *face = createFace();
  face->m_vertexCount = vertexCount;
  face->m_hedge = edges[0]->getHedge();
  for(int i=0;i<vertexCount;i++)
  {
    Vertex *v0 = vertices[i];
    Vertex *v1 = vertices[(i+1)%vertexCount];
    HEdge *e0 = edges[i]->getHedge();
    HEdge *e1 = e0->getTween();
    v0->m_edgeCount = 2;
    v0->m_hedge = e0;
    e0->linkTo(edges[(i+1)%vertexCount]->getHedge());
    e0->m_leftFace = face;
    e0->m_nextVertex = v1;
    e1->linkTo(edges[(i+vertexCount-1)%vertexCount]->getHedge()->getTween());
    e1->m_leftFace = 0;
    e1->m_nextVertex = v0;
  }
  return face;
}

//-----------------------------------------------------------------------------

Net::Face *Net::growFace(int vertexCount, int iv0, int iv1)
{
  // create new face. set vertexCount
  Face *face = createFace();
  face->m_vertexCount = vertexCount;

  // check v0, v1
  assert(0<=iv0 && iv0<getVertexCount());
  assert(0<=iv1 && iv1<getVertexCount());
  assert(iv0!=iv1);
  Vertex *v0 = getVertex(iv0);
  Vertex *v1 = getVertex(iv1);

  // increment edgeCount
  v0->m_edgeCount++;
  v1->m_edgeCount++;

  // find boundary half edge entering into v0 (i.e. the prev half edge)
  HEdge *he0 = v0->getHedge();
  HEdge *he = he0;
  for(;;)
  {
    assert(he->getPrevVertex()==v0);
    if(he->getRightFace() == 0) break;
    he = he->getTween()->getNext();
    if(he==he0) {assert(0); return 0;}
  }
  HEdge *prevHalfEdge = he->getTween();
  assert(prevHalfEdge->getNextVertex() == v0);
  he = prevHalfEdge->getNext();
  HEdge *firstHalfEdge = face->m_hedge = he;
  int m = vertexCount-2;
  for(;;)
  {
    he->m_leftFace = face;
    if(he->getNextVertex() == v1) break;
    m--;
    he = he->getNext();
    if(he->getNextVertex() == v0) {assert(0); return 0;}
  }
  HEdge *lastHalfEdge = he;
  HEdge *nextHalfEdge = he->getNext();

  std::vector<Vertex*> newVertices(m);
  std::vector<Edge*> newEdges(m+1);
  for(int i=0;i<m;i++) newVertices[i] = createVertex();
  for(int i=0;i<m+1;i++) newEdges[i] = createEdge();
  for(int i=0;i<m+1;i++)
  {
    Edge *edge = newEdges[i];
    Vertex *va = i==0 ? v1 : newVertices[i-1];
    Vertex *vb = i==m ? v0 : newVertices[i];

    HEdge *e0 = edge->getHedge();
    e0->m_leftFace = face;
    e0->m_nextVertex = vb;
    if(i<m) e0->linkTo(newEdges[i+1]->getHedge());

    HEdge *e1 = e0->getTween();
    e1->m_leftFace = 0;
    e1->m_nextVertex = va;
    if(i>0) e1->linkTo(newEdges[i-1]->getHedge()->getTween());
  }
  for(int i=0;i<m;i++)
  {
    Vertex *vertex = newVertices[i];
    vertex->m_edgeCount = 2;
    vertex->m_hedge = newEdges[i+1]->getHedge();
  }

  prevHalfEdge->linkTo(newEdges[m]->getHedge()->getTween());
  newEdges[0]->getHedge()->getTween()->linkTo(nextHalfEdge);
  lastHalfEdge->linkTo(newEdges[0]->getHedge());
  newEdges[m]->getHedge()->linkTo(firstHalfEdge);

  return face;
}

//-----------------------------------------------------------------------------

void Net::destroyFace(Face *face)
{
  int faceIndex = face->getIndex();
  m_faces.erase(m_faces.begin()+faceIndex);
  for(int i=faceIndex;i<getFaceCount();i++)
    m_faces[i]->m_index = i;
  HEdge *he = face->getHedge();
  std::vector<HEdge*> hedges;
  do {
    hedges.push_back(he);
    he->m_leftFace = 0;
    he = he->getNext();
  } while(he != face->getHedge());
  delete face;
  int n = (int)hedges.size();
  for(int i=0;i<n;i++)
  {
    int i1 = (i+1)%n;
    HEdge *he = hedges[i];
    HEdge *he1 = hedges[i1];
    Vertex *v = he->getNextVertex();
    if(he->getRightFace() == 0 && he1->getRightFace() == 0)
    {
      // kill edge & next edge => kill vertex
      int vertexIndex = v->getIndex();
      m_vertices.erase(m_vertices.begin()+vertexIndex);
      for(int j=vertexIndex;j<(int)m_vertices.size();j++)
        m_vertices[j]->m_index = j;
      delete v;
    }
    else if(he->getRightFace() == 0)
    {
      // kill edge
      v->m_edgeCount--;
      if(v->m_hedge == he->m_tween) v->m_hedge = he1;
      he->m_tween->m_prev->linkTo(he1);
    }
    else if(he1->getRightFace() == 0)
    {
      // kill next edge
      v->m_edgeCount--;
      if(v->m_hedge == he1) v->m_hedge = he->m_tween;
      he->linkTo(he1->m_tween->m_next);
    }
  }
  for(int i=0;i<n;i++)
  {
    HEdge *he = hedges[i];
    if(he->getRightFace() == 0)
    {
      HEdge *he1 = he->m_tween;
      Edge *e = he->getEdge();
      int i0 = he->m_index;
      int i1 = he1->m_index;
      if(i0>i1) {int ii=i0;i0=i1;i1=ii;}
      m_hedges.erase(m_hedges.begin()+i1);
      m_hedges.erase(m_hedges.begin()+i0);
      for(int j=i0;j<(int)m_hedges.size();j++)
        m_hedges[j]->m_index = j;
      m_edges.erase(m_edges.begin()+e->m_index);
      for(int j=e->m_index;j<(int)m_edges.size();j++)
        m_edges[j]->m_index = j;
      delete e;
      delete he;
      delete he1;
    }
  }
}

//-----------------------------------------------------------------------------

void Net::getFaceVertices(std::vector<Vertex*> &vertices, const Face *face) const
{
  HEdge *he = face->getHedge();
  do {
    vertices.push_back(he->getNextVertex());
    he = he->getNext();
  } while(he != face->getHedge());
}

//-----------------------------------------------------------------------------

void Net::getFaceEdges(std::vector<Edge*> &edges, const Face *face) const
{
  HEdge *he = face->getHedge();
  do {
    edges.push_back(he->getEdge());
    he = he->getNext();
  } while(he != face->getHedge());
}

//-----------------------------------------------------------------------------

void Net::getFaceHedges(std::vector<HEdge*> &hedges, const Face *face) const
{
  HEdge *he = face->getHedge();
  do {
    hedges.push_back(he);
    he = he->getNext();
  } while(he != face->getHedge());
}

//-----------------------------------------------------------------------------

void Net::getAdjacentVertices(std::vector<Vertex*> &vertices, const Vertex *vertex) const
{
  HEdge *he = vertex->getHedge();
  do {
    vertices.push_back(he->getNextVertex());
    he = he->getTween()->getNext();
  } while(he != vertex->getHedge());
}

//-----------------------------------------------------------------------------

void Net::getAdjacentEdges(std::vector<Edge*> &edges, const Vertex *vertex) const
{
  HEdge *he = vertex->getHedge();
  do {
    edges.push_back(he->getEdge());
    he = he->getTween()->getNext();
  } while(he != vertex->getHedge());
}

//-----------------------------------------------------------------------------

void Net::getOutHedges(std::vector<HEdge*> &hedges, const Vertex *vertex) const
{
  HEdge *he = vertex->getHedge();
  if(!he) return;
  do {
    hedges.push_back(he);
    he = he->getTween()->getNext();
  } while(he != vertex->getHedge());
}

//-----------------------------------------------------------------------------

void Net::getInHedges(std::vector<HEdge*> &hedges, const Vertex *vertex) const
{
  HEdge *he = vertex->getHedge();
  if(!he) return;
  do {
    hedges.push_back(he->getTween());
    he = he->getTween()->getNext();
  } while(he != vertex->getHedge());
}

//-----------------------------------------------------------------------------

Net::HEdge *Net::getHedge(const Vertex *v0, const Vertex *v1) const
{
  HEdge *he = v0->getHedge();
  if(!he) return 0;
  do {
    if(he->getNextVertex() == v1) return he;
    he = he->getTween()->getNext();
  } while(he != v0->getHedge());
  return 0;
}

//-----------------------------------------------------------------------------

Net::HEdge *Net::getHedge(const Vertex *v, const Face *f) const
{
  HEdge *he = v->getHedge();
  if(!he) return 0;
  do {
    if(he->getLeftFace() == f) return he;
    he = he->getTween()->getNext();
  } while(he != v->getHedge());
  return 0;
}

//-----------------------------------------------------------------------------

Net::HEdge *Net::getHedge(const Face *f0, const Face *f1) const
{
  HEdge *he = f0->getHedge();
  if(!he) return 0;
  do {
    if(he->getRightFace() == f1) return he;
    he = he->getNext();
  } while(he != f0->getHedge());
  return 0;
}

//-----------------------------------------------------------------------------

void Net::cut(HEdge *he)
{
  Edge *e = createEdge();

  e->getHedge()->m_nextVertex = he->getNextVertex();
  e->getHedge()->getTween()->m_nextVertex = he->getPrevVertex();
  he->getNextVertex()->m_edgeCount++;
  he->getPrevVertex()->m_edgeCount++;

  e->getHedge()->linkTo(he->getNext());
  he->getPrev()->linkTo(e->getHedge());
  e->getHedge()->getTween()->linkTo(he);
  he->linkTo(e->getHedge()->getTween());


  e->getHedge()->m_leftFace = he->getLeftFace();
  e->getHedge()->m_leftFace->m_hedge = e->getHedge();
  e->getHedge()->getTween()->m_leftFace = 0;
  he->m_leftFace = 0;

  split(he->getNextVertex());
  split(he->getPrevVertex());
}

//-----------------------------------------------------------------------------

void Net::cut(Face *f, Vertex *v0, Vertex *v1)
{
  HEdge *he0 = v0->getHedge();
  do {
    if(he0->getLeftFace() == f) break;
    he0 = he0->getTween()->getNext();
  } while(he0 != v0->getHedge());
  assert(he0->getLeftFace() == f);
  HEdge *he1 = v1->getHedge();
  do {
    if(he1->getLeftFace() == f) break;
    he1 = he1->getTween()->getNext();
  } while(he1 != v1->getHedge());
  assert(he1->getLeftFace() == f);

  Edge *e = createEdge();
  e->getHedge()->m_nextVertex = v0;
  e->getHedge()->getTween()->m_nextVertex = v1;
  he0->getPrev()->linkTo(e->getHedge()->getTween());
  he1->getPrev()->linkTo(e->getHedge());
  e->getHedge()->linkTo(he0);
  e->getHedge()->getTween()->linkTo(he1);
  v0->m_edgeCount++;
  v1->m_edgeCount++;
  Face *f1 = createFace();
  HEdge *he = he0;
  do {
    he->m_leftFace = f1;
    f1->m_vertexCount++;
    f->m_vertexCount--;
    he = he->getNext();
  } while(he != he0);
  f1->m_hedge = he0;
  f->m_hedge = he1;
  f->m_vertexCount+=2;
  e->getHedge()->m_tween->m_leftFace = f;
  cut(e->getHedge());
}

//-----------------------------------------------------------------------------

void Net::split(Vertex *v)
{
  HEdge *he = v->getHedge();
  do {
    if(he->getLeftFace() == 0) break;
    he = he->getTween()->getNext();
  } while(he != v->getHedge());
  if(he->getLeftFace() != 0) return;

  HEdge *he0 = he->getPrev();
  HEdge *he1 = he;
  he = he->getTween()->getNext();
  do {
    if(he->getLeftFace() == 0) break;
    he = he->getTween()->getNext();
  } while(he != he1);
  if(he->getLeftFace() != 0 || he==he1) return;

  HEdge *he2 = he->getPrev();
  HEdge *he3 = he;

  Vertex *v1 = createVertex();

  he0->linkTo(he3);
  he2->linkTo(he1);

  v->m_hedge = he3;
  v1->m_hedge = he1;

  he = he1;
  do {
    assert(he->getTween()->m_nextVertex == v);
    he->getTween()->m_nextVertex = v1;
    v1->m_edgeCount++;
    v->m_edgeCount--;
    he = he->getTween()->getNext();
  } while(he != he1);

  assert(v->getHedge()->getPrevVertex() == v);
  assert(v1->getHedge()->getPrevVertex() == v1);
}

//-----------------------------------------------------------------------------

void Net::checkIntegrity()
{
  for(int i=0;i<getHEdgeCount();i++)
  {
    HEdge *he = getHEdge(i);
    assert(he->m_index == i);
    assert(he->m_edge->m_hedge == he || he->m_edge->m_hedge == he->getTween());
    assert(he->m_tween != he);
    assert(he->m_tween->m_tween == he);
    assert(he->m_next != he);
    assert(he->m_next->m_prev == he);
    assert(he->m_prev != he);
    assert(he->m_prev->m_next == he);
  }

  for(int i=0;i<getFaceCount();i++)
  {
    Face *f = getFace(i);
    assert(f->m_index == i);
    int count=0;
    HEdge *he = f->getHedge();
    do {
      count++;
      assert(he->m_leftFace == f);
      he = he->getNext();
    } while(he != f->getHedge());
    assert(f->m_vertexCount == count);
  }

  for(int i=0;i<getEdgeCount();i++)
  {
    Edge *e = getEdge(i);
    assert(e->m_index == i);
    assert(e->m_hedge->m_edge==e);
  }

  for(int i=0;i<getVertexCount();i++)
  {
    Vertex *v = getVertex(i);
    assert(v->m_index == i);
    HEdge *he = v->getHedge();
    if(he)
    {
      int count=0;
      do {
        count++;
        assert(he->getPrevVertex()==v);
        he = he->getTween()->getNext();
      } while(he != v->getHedge());
      assert(v->m_edgeCount == count);
    }
    else
    {
      assert(v->m_edgeCount == 0);
    }
  }
}

//-----------------------------------------------------------------------------

/*
template <class Trait>
Net::Net();
virtual ~Net();

inline Vertex *createVertex() {return add(new Vertex());}
inline Edge   *createEdge()   {return add(new Edge());}
inline HEdge  *createHEdge()  {return add(new HEdge());}
inline Face   *createFace()   {return add(new Face());}

Face *createFace(const int vertexIndices[], int vertexCount);
inline Face *createFace(const std::vector<int> &vv) {return createFace(&vv[0],(int)vv.size());}
inline Face *createFace(int a, int b, int c) {int vv[] = {a,b,c}; return createFace(vv,3);}
inline Face *createFace(int a, int b, int c, int d)  {int vv[] = {a,b,c,d}; return createFace(vv,4);}
inline Face *createFace(int a, int b, int c, int d, int e)  {int vv[] = {a,b,c,d,e}; return createFace(vv,5);}

Face *createFirstFace(int vertexCount);
Face *growFace(int vertexCount, int v0, int v1);

void destroyFace(Face *face);

void getFaceVertices(std::vector<Vertex*> &vertices, const Face *face) const;
void getFaceEdges(std::vector<Edge*> &edges, const Face *face) const;
void getFaceHedges(std::vector<HEdge*> &hedges, const Face *face) const;

inline void getFaceVertices(std::vector<Vertex*> &vertices, int faceIndex) const {getFaceVertices(vertices, getFace(faceIndex));}
inline void getFaceEdges(std::vector<Edge*> &edges, int faceIndex) const {getFaceEdges(edges, getFace(faceIndex));}
inline void getFaceHedges(std::vector<HEdge*> &hedges, int faceIndex) const {getFaceHedges(hedges, getFace(faceIndex));}

void getAdjacentVertices(std::vector<Vertex*> &vertices, const Vertex *vertex) const;
void getAdjacentEdges(std::vector<Edge*> &edges, const Vertex *vertex) const;
void getOutHedges(std::vector<HEdge*> &hedges, const Vertex *vertex) const;
void getInHedges(std::vector<HEdge*> &hedges, const Vertex *vertex) const;

inline void getAdjacentVertices(std::vector<Vertex*> &vertices, int vertexIndex) const {getAdjacentVertices(vertices, getVertex(vertexIndex));}
inline void getAdjacentEdges(std::vector<Edge*> &edges, int vertexIndex) const {getAdjacentEdges(edges, getVertex(vertexIndex));}
inline void getOutHedges(std::vector<HEdge*> &hedges, int vertexIndex) const {getOutHedges(hedges, getVertex(vertexIndex));}
inline void getInHedges(std::vector<HEdge*> &hedges, int vertexIndex) const {getInHedges(hedges, getVertex(vertexIndex));}

inline int getVertexCount() const {return (int)m_vertices.size();}
inline int getHedgeCount() const {return (int)m_hedges.size();}
inline int getEdgeCount() const {return (int)m_edges.size();}
inline int getFaceCount() const {return (int)m_faces.size();}

inline Vertex *getVertex(int index) const {return m_vertices[index];}
inline Edge *getEdge(int index) const {return m_edges[index];}
inline HEdge *getHedge(int index) const {return m_hedges[index];}
inline Face *getFace(int index) const {return m_faces[index];}

HEdge *getHedge(const Vertex *v0, const Vertex *v1) const; // hedge: v0->v1
HEdge *getHedge(const Vertex *v, const Face *f) const; // hedge: v->, face=f
HEdge *getHedge(const Face *f0, const Face *f1) const;

void cut(HEdge *he);
void cut(Face *f, Vertex *va, Vertex *vb);
void split(Vertex *v);

void checkIntegrity();
*/

#ifdef DOPO
//-----------------------------------------------------------------------------


#endif

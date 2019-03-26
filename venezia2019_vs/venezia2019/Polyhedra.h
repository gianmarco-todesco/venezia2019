#ifndef POLYHEDRA_INCLUDED
#define POLYHEDRA_INCLUDED

#include <QVector3D>
#include <qmatrix4x4.h>
#include <assert.h>

class Polyhedron {
public:
  typedef std::vector<int> Indices;
  class Vertex {
  public:
    QVector3D m_pos;
    int m_status;
    Vertex(const QVector3D &pos, int status=0) : m_pos(pos), m_status(status) {}
  };
  class Edge {
  public:
    int m_a, m_b;
    int m_status;
    Edge(int a, int b, int status=0) : m_a(a<b?a:b), m_b(a<b?b:a), m_status(status) { }
  };
  class Face {
  public:
    Indices m_edges;
    mutable Indices m_vertices;
    int m_status;
    Face(const Indices &e, int status=0) 
    : m_edges(e), m_vertices(), m_status(status) {}
  };

  Polyhedron(const std::vector<Vertex> &v, 
             const std::vector<Edge> &e, 
             const std::vector<Face> &f);
  ~Polyhedron() {}

  int getVertexCount() const {return m_vertices.size();}
  Vertex &getVertex(int index)  {
    assert(0<=index && index<getVertexCount());
    return m_vertices[index];
  }
  const Vertex &getVertex(int index) const {
    assert(0<=index && index<getVertexCount());
    return m_vertices[index];
  }
  int getEdgeCount() const {return m_edges.size();}
  Edge &getEdge(int index)  {
    assert(0<=index && index<getEdgeCount());
    return m_edges[index];
  }
  const Edge &getEdge(int index) const {
    assert(0<=index && index<getEdgeCount());
    return m_edges[index];
  }
  int getFaceCount() const {return m_faces.size();}
  Face &getFace(int index)  {
    assert(0<=index && index<getFaceCount());
    return m_faces[index];
  }
  const Face &getFace(int index) const {
    assert(0<=index && index<getFaceCount());
    return m_faces[index];
  }
  
  void computeFaceVertices() const;

  bool isValid() const;
  bool areFaceVerticesValid() const { 
    return m_validFaceVertices;
  }

  void scale(double scaleFactor) {
      for(int i=0;i<(int)m_vertices.size();i++) 
          m_vertices[i].m_pos *= scaleFactor;
  }


private:
  
  std::vector<Vertex> m_vertices;
  std::vector<Edge> m_edges;
  std::vector<Face> m_faces;
  mutable bool m_validFaceVertices;

public:
  int m_status;
  int m_shell;
  unsigned long m_planeMask;

  void serialize(std::ostream &out);
  static Polyhedron *deserialize(std::istream &in);

};


typedef std::vector<Polyhedron*> Polyhedra;



// Plane getFacePlane(const Polyhedron*ph, int index);

QVector3D getFaceNormal(const Polyhedron*ph, int index);
QVector3D getFaceCenter(const Polyhedron*ph, int index);
std::pair<QVector3D, QVector3D> 
  getFaceCenterAndNormal(const Polyhedron*ph, int index);

QMatrix4x4 getFaceMatrix(const Polyhedron*ph, int index);

void getFaceVertices(Polyhedron::Indices &result,const Polyhedron*ph, int index);
void getVertexFaces(Polyhedron::Indices &result,const Polyhedron*ph, int index);
bool isFaceSmall(const Polyhedron*ph, int index);

// find indices of the vertices adjacent to the index-th vertex. 
// the order is ccw looking at the origin
void getAdjacentVertices(Polyhedron::Indices &indices, const Polyhedron*ph, int index);

QVector3D getCenter(const Polyhedron*ph);

void transform(Polyhedron *ph, const QMatrix4x4 &matrix);


Polyhedron *makePolyhedron(
  const std::vector<QVector3D> &pts, 
  const std::vector<int> &faces);

// definiti in resourcemanager.cpp
Polyhedron *makeTetrahedron();
Polyhedron *makeEsahedron();
Polyhedron *makeOctahedron();
Polyhedron *makeDodecahedron();
Polyhedron *makeIcosahedron();

inline Polyhedron *makeCube() {return makeEsahedron();}

Polyhedron *makeDual(const Polyhedron *src);

Polyhedron *rectificate(const Polyhedron *src);
Polyhedron *truncate(const Polyhedron *src, double t); // 0<t<0.5

Polyhedron *makeCuboctahedron();
Polyhedron *makeTruncatedDodecahedron();


#endif

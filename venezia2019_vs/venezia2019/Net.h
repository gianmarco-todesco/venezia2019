#ifndef NET_H
#define NET_H

#include <vector>

namespace GmLib {

  class Net {
  public:

    class HEdge;
    class Edge;
    class Face;

    class Vertex {
      friend class Net;
      int m_index;
      HEdge *m_hedge; // an HEdge coming out from the vertex
      int m_edgeCount;      
    public:
      Vertex() : m_index(0), m_hedge(0), m_edgeCount(0) {}
      virtual ~Vertex() {}
      inline int getIndex() const {return m_index;}
      inline HEdge *getHedge() const {return m_hedge;}
      inline int getEdgeCount() const {return m_edgeCount;}
    };

    class HEdge {
      friend class Net;
      int m_index;
      Edge *m_edge;
      HEdge *m_next,*m_prev,*m_tween;
      Vertex *m_nextVertex;
      Face *m_leftFace;
      inline void linkTo(HEdge *hedge) {m_next = hedge;hedge->m_prev=this;}

    public:
      HEdge() : m_index(0), m_edge(0), m_next(0), m_prev(0), m_tween(0)
              , m_nextVertex(0), m_leftFace(0) {}
      virtual ~HEdge() {}
      inline int getIndex() const {return m_index;}
      inline Edge *getEdge() const {return m_edge;}
      inline HEdge *getNext() const {return m_next;}
      inline HEdge *getPrev() const {return m_prev;}
      inline HEdge *getTween() const {return m_tween;}
      inline Vertex *getNextVertex() const {return m_nextVertex;}
      inline Vertex *getPrevVertex() const {return m_tween->m_nextVertex;}
      inline Face *getLeftFace() const {return m_leftFace;}
      inline Face *getRightFace() const {return m_tween->m_leftFace;}
    };

    class Edge {
      friend class Net;
      int m_index;
      HEdge *m_hedge;
    public:
      Edge() : m_index(0), m_hedge(0) {}
      virtual ~Edge() {}
      inline int getIndex() const {return m_index;}
      inline HEdge *getHedge() const {return m_hedge;}
      inline Vertex *getV0() const {return m_hedge->getNextVertex();}
      inline Vertex *getV1() const {return m_hedge->getTween()->getNextVertex();}
    };

    class Face {
      friend class Net;
      int m_index;
      HEdge *m_hedge;
      int m_vertexCount;
    public:
      Face() : m_index(0), m_hedge(0), m_vertexCount(0) {}
      virtual ~Face() {}
      inline int getIndex() const {return m_index;}
      inline HEdge *getHedge() const {return m_hedge;}
      inline int getVertexCount() const {return m_vertexCount;}
      void setHedge(HEdge *he) {m_hedge = he;}
    };

    Net();
    virtual ~Net();

    void clear();

    virtual Vertex *createVertex() { return add(new Vertex()); }
    virtual Edge   *createEdge()   { return add(new Edge()); }
    virtual HEdge  *createHEdge()  { return add(new HEdge()); }
    virtual Face   *createFace()   { return add(new Face()); }

    void createVertices(int count);

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
    int getAdjacentVertexCount(const Vertex *vertex) const {
      std::vector<Vertex*> vertices; getAdjacentVertices(vertices, vertex);
      return (int)vertices.size();
    }

    void getAdjacentEdges(std::vector<Edge*> &edges, const Vertex *vertex) const;
    void getOutHedges(std::vector<HEdge*> &hedges, const Vertex *vertex) const;
    void getInHedges(std::vector<HEdge*> &hedges, const Vertex *vertex) const;

    inline void getAdjacentVertices(std::vector<Vertex*> &vertices, int vertexIndex) const {getAdjacentVertices(vertices, getVertex(vertexIndex));}
    inline void getAdjacentEdges(std::vector<Edge*> &edges, int vertexIndex) const {getAdjacentEdges(edges, getVertex(vertexIndex));}
    inline void getOutHedges(std::vector<HEdge*> &hedges, int vertexIndex) const {getOutHedges(hedges, getVertex(vertexIndex));}
    inline void getInHedges(std::vector<HEdge*> &hedges, int vertexIndex) const {getInHedges(hedges, getVertex(vertexIndex));}

    inline int getVertexCount() const {return (int)m_vertices.size();}
    inline int getHEdgeCount() const {return (int)m_hedges.size();}
    inline int getEdgeCount() const {return (int)m_edges.size();}
    inline int getFaceCount() const {return (int)m_faces.size();}

    inline Vertex *getVertex(int index) const {return m_vertices[index];}
    inline Edge *getEdge(int index) const {return m_edges[index];}
    inline HEdge *getHEdge(int index) const {return m_hedges[index];}
    inline Face *getFace(int index) const {return m_faces[index];}

    HEdge *getHedge(const Vertex *v0, const Vertex *v1) const; // hedge: v0->v1
    HEdge *getHedge(const Vertex *v, const Face *f) const; // hedge: v->, face=f
    HEdge *getHedge(const Face *f0, const Face *f1) const;

    void cut(HEdge *he);
    void cut(Face *f, Vertex *va, Vertex *vb);
    void split(Vertex *v);

    void checkIntegrity();

  protected:
    Vertex *add(Vertex*);
    Edge   *add(Edge*);
    HEdge  *add(HEdge*);
    Face   *add(Face*);

  private:
    std::vector<Vertex*> m_vertices;
    std::vector<Edge*> m_edges;
    std::vector<HEdge*> m_hedges;
    std::vector<Face*> m_faces;
  }; //  class Net

#ifdef CICCIO
  //---------------------------------------------------------------------------

  struct NetTraits {
    struct VertexData {};
    struct HEdgeData {};
    struct EdgeData {};
    struct FaceData {};
  };

  //---------------------------------------------------------------------------

  template<class Traits>
  class Net : public NetBase {
  public:
    typedef typename Traits::VertexData VertexData;
    typedef typename Traits::HEdgeData HEdgeData;
    typedef typename Traits::EdgeData EdgeData;
    typedef typename Traits::FaceData FaceData;
    struct VertexWrapper : public Vertex { VertexData d;};
    struct HEdgeWrapper : public HEdge { HEdgeData d;};
    struct EdgeWrapper : public Edge { EdgeData d;};
    struct FaceWrapper : public Face { FaceData d;};
  public:
    Net() {}

    Vertex *createVertex() {return add(new VertexWrapper());}
    HEdge  *createHEdge()  {return add(new HEdgeWrapper());}
    Edge   *createEdge()   {return add(new EdgeWrapper());}
    Face   *createFace()   {return add(new FaceWrapper());}

    VertexData *getVertexData(Vertex*v) const { return &(static_cast<VertexWrapper*>(v)->d); }
    VertexData *getVertexData(int index) const { return getVertexData(getVertex(index));}

    FaceData *getFaceData(Face*f) const { return &(static_cast<FaceWrapper*>(f)->d); }
    FaceData *getFaceData(int index) const { return getFaceData(getFace(index));}

    HEdgeData *getHEdgeData(HEdge*he) const { return &(static_cast<HEdgeWrapper*>(he)->d); }
    HEdgeData *getHEdgeData(int index) const { return getHEdgeData(getHEdge(index));}
    /*
    HEdgeData *data(HEdge*he) const { return static_cast<HEdgeWrapper*>(he)->d; }
    const HEdgeData *data(const HEdge*he) const { return static_cast<HEdgeWrapper*>(he)->d; }

    EdgeData *data(Edge*e) const { return static_cast<EdgeWrapper*>(e)->d; }
    const EdgeData *data(const Edge*e) const { return static_cast<EdgeWrapper*>(e)->d; }

    FaceData *data(Face*f) const { return static_cast<FaceWrapper*>(f)->d; }
    const FaceData *data(const Face*f) const { return static_cast<FaceWrapper*>(f)->d; }
*/

  };

#endif


} // namespace GmLib


#endif


#pragma warning(disable:4786)

#include "polyhedra.h"
#include <map>
#include <set>
#include <algorithm>
// #include "util.h"

#define _USE_MATH_DEFINES
#include <math.h>

const double epsilon = 1.0e-4;

//-------------------------------------------------------------------

Polyhedron::Polyhedron(
  const std::vector<Vertex> &v, 
  const std::vector<Edge> &e, 
  const std::vector<Face> &f)
  : m_vertices(v)
  , m_edges(e)
  , m_faces(f)
  , m_status(0)
  , m_shell(0)
  , m_planeMask(0)
  , m_validFaceVertices(false)
{
  // assert(isValid());
}


//-------------------------------------------------------------------

Polyhedron *makePolyhedron(
  const std::vector<QVector3D> &pts, 
  const std::vector<int> &faces)
{
  int vCount = pts.size();
  std::vector<Polyhedron::Vertex> v;
  std::vector<Polyhedron::Edge> e;
  std::vector<Polyhedron::Face> f;
  for(int i=0;i<vCount;i++)
    v.push_back(Polyhedron::Vertex(pts[i]));
  std::map<int,int> edgeTable;
  int n = faces.size();
  for(int i=0;i<n;)
    {
     int m = faces[i++];
     assert(m>=3);
     assert(i+m<=n);
     std::vector<int> face;
     for(int j=0;j<m;j++)
       {
        int va = faces[i+j];
        int vb = faces[i+((j+1)%m)];
        assert(0<=va && va<vCount);
        assert(0<=vb && vb<vCount);
        assert(va!=vb);
        int id = va<vb ? va*vCount+vb : vb*vCount+va;
        std::map<int,int>::iterator it = edgeTable.find(id);
        if(it != edgeTable.end()) 
          face.push_back(it->second);
        else {
          int index = e.size();
          face.push_back(index);
          e.push_back(Polyhedron::Edge(va,vb));
          edgeTable[id] = index;
         }
       }
     f.push_back(Polyhedron::Face(face));
     i+=m;
    }
  return new Polyhedron(v,e,f);
}


//-------------------------------------------------------------------

bool Polyhedron::isValid() const 
{
  int vCount = getVertexCount();
  if(vCount<4) {assert(0); return false;}
  if(getEdgeCount()<6) {assert(0); return false;}
  if(getFaceCount()<4) {assert(0); return false;}
  int i;
  for(i=0;i<getEdgeCount();i++)
    {
     const Edge &e = getEdge(i);
     if(!(0<=e.m_a && e.m_a<e.m_b && e.m_b<vCount)) {assert(0); return false;}
    }
  for(i=0;i<getFaceCount();i++)
    {
     const Face &f = getFace(i);
     int m=f.m_edges.size();
     if(m<3){assert(0); return false;}
     for(int j=0;j<m;j++)
       {
        int k = f.m_edges[j];
        if(!(0<=k && k<getEdgeCount())) {assert(0); return false;}
        if(std::find(f.m_edges.begin(), f.m_edges.begin()+j, k) != f.m_edges.begin()+j)
          {assert(0); return false;}
       }
    }
  return true;
}


//-------------------------------------------------------------------

void checkOrientation(Polyhedron *ph)
{
    qDebug() << "--------------------------";
    for(int i=0;i<ph->getFaceCount();i++)
    {
        Polyhedron::Indices vv;
        getFaceVertices(vv, ph, i);
        QVector3D fc = getFaceCenter(ph, i);
        QList<QVector3D> pts;
        int m = (int)vv.size();
        assert(m>=3);
        for(int j=0;j<m;j++)
        {
            QVector3D p = ph->getVertex(vv[j]).m_pos;
            pts.append(p);
        }
        for(int j=0; j<m;j++)
        {
            QVector3D u = QVector3D::crossProduct(pts[(j+1)%m]-fc, pts[j]-fc);
            // assert(QVector3D::dotProduct(u, fc)>0);
            if(QVector3D::dotProduct(u, fc)<=0)
                qDebug() << i << j;
        }
    }
}

//-------------------------------------------------------------------

void unroll(
  std::vector<int> &result,
  std::vector<std::pair<int, int> > links)
{
  result.clear();
  if(links.empty()) return;
  int a = links.back().first;
  int b = links.back().second;
  result.push_back(a);
  if(links.size()==1)
    {
     assert(a==b);
    }
  else
    {
     assert(a!=b);
     links.pop_back();
     while(a!=b)
       {
        int m = links.size();
        int j;
        for(j=0;j<m;j++)
          if(links[j].first == a)
            {result.push_back(a = links[j].second); break;}
          else if(links[j].second == a)
            {result.push_back(a = links[j].first); break;}
        assert(j<m); 
        if(j<m-1) links[j] = links[m-1];
        links.pop_back();
       }     
    }  
}

//-------------------------------------------------------------------

void getFaceVertices(Polyhedron::Indices &result, const Polyhedron*ph, int index)
{
  const Polyhedron::Face &f = ph->getFace(index);
  std::vector<std::pair<int,int> > links;
  for(int i=0;i<(int)f.m_edges.size();i++)
    {
     const Polyhedron::Edge &e = ph->getEdge(f.m_edges[i]);
     links.push_back(std::pair<int,int>(e.m_a,e.m_b));    
    }
  unroll(result,links);
  assert(result.size()==f.m_edges.size());
}

//-------------------------------------------------------------------

void getVertexFaces(Polyhedron::Indices &result,const Polyhedron*ph, int vi)
{
  const Polyhedron::Vertex &v = ph->getVertex(vi);
  std::vector<std::pair<int,int> > links;
  std::map<int, std::pair<int, int> > edges;
  std::map<int, std::pair<int, int> >::iterator it;
  for(int fi=0;fi<ph->getFaceCount();fi++)
  {
    const Polyhedron::Face &f = ph->getFace(fi);
    for(int j=0;j<(int)f.m_edges.size();j++)
    {
      int ei = f.m_edges[j];
      const Polyhedron::Edge &e = ph->getEdge(ei);
      if(e.m_a == vi || e.m_b == vi)
      {
        it = edges.find(ei);
        if(it == edges.end()) edges[ei] = std::make_pair(fi,-1);
        else {it->second.second = fi;links.push_back(it->second);}
      }
    }
  }
  unroll(result,links);
}



//-------------------------------------------------------------------

void Polyhedron::computeFaceVertices() const
{
  QVector3D phCenter = getCenter(this);
  for(int i=0;i<getFaceCount();i++)
    {
     const Face &f = getFace(i);
     Indices vv;
     getFaceVertices(vv, this, i);
     int m = vv.size();
     assert(vv.size()>=3);
     QVector3D fc;
     for(int j=0;j<m;j++) 
       fc += getVertex(vv[j]).m_pos;
     fc /= m;
     QVector3D p0 = getVertex(vv[0]).m_pos;
     QVector3D p1 = getVertex(vv[1]).m_pos;

     if((p0-p1).length()>1e-4)
     {
        QVector3D normal = QVector3D::crossProduct(p0-fc,p1-p0);
        assert(normal.lengthSquared()>0);
        if(QVector3D::dotProduct(normal, fc-phCenter)<0)
          std::reverse(vv.begin(), vv.end());
     }
     f.m_vertices = vv;
  }
  m_validFaceVertices = true;
}


//-------------------------------------------------------------------


QVector3D getNormal(const Polyhedron*ph, Polyhedron::Indices &vv)
{
  int i;
  int m = vv.size();
  assert(m>=3);
  QVector3D fc;
  for(i=0;i<m;i++)
    fc+=ph->getVertex(vv[i]).m_pos;
  fc/=m;
  QVector3D normal;
  for(i=0;i<m;i++)
    {
     QVector3D p0 = ph->getVertex(vv[i]).m_pos;  
     QVector3D p1 = ph->getVertex(vv[(i+1)%m]).m_pos;
     normal += QVector3D::crossProduct(p0-fc,p1-p0);
    }
  assert(normal.length()>epsilon);
  normal.normalize();
  QVector3D pc = getCenter(ph);
  double d = QVector3D::dotProduct(normal, fc-pc);
  if(d<0) normal = -normal;

  #ifndef NDEBUG
  for(i=0;i<(int)vv.size();i++)
    {
     QVector3D p = ph->getVertex(vv[i]).m_pos; 
     d = QVector3D::dotProduct(normal, p-fc);
     assert(d<10*epsilon); 
    }
  #endif
  return normal;
}

//-------------------------------------------------------------------

QVector3D getFaceNormal(const Polyhedron*ph, int index)
{
  Polyhedron::Indices vv;
  getFaceVertices(vv, ph, index);
  return getNormal(ph, vv);
}

//-------------------------------------------------------------------

QVector3D getFaceCenter(const Polyhedron*ph, int index)
{
  Polyhedron::Indices vv;
  getFaceVertices(vv, ph, index);
  int m = vv.size();
  assert(m>=3);
  QVector3D c;
  for(int i=0;i<m;i++) 
    c+=ph->getVertex(vv[i]).m_pos;
  c/=m;
  return c;
}

//-------------------------------------------------------------------

std::pair<QVector3D, QVector3D> 
getFaceCenterAndNormal(const Polyhedron*ph, int index)
{
  Polyhedron::Indices vv;
  getFaceVertices(vv, ph, index);
  int m = vv.size();
  assert(m>=3);
  QVector3D fc;
  for(int i=0;i<m;i++) 
    fc+=ph->getVertex(vv[i]).m_pos;
  fc/=m;
  QVector3D normal = getNormal(ph, vv);
  return std::make_pair<QVector3D,QVector3D>(fc,normal);
}

//-------------------------------------------------------------------

QMatrix4x4 getFaceMatrix(const Polyhedron*ph, int index)
{
    std::pair<QVector3D, QVector3D> cn = getFaceCenterAndNormal(ph, index);
    const Polyhedron::Face &face = ph->getFace(index);
    QVector3D p = ph->getVertex(ph->getEdge(face.m_edges[0]).m_a).m_pos;

    QVector3D fc = cn.first;
    QVector3D e2 = cn.second;
    QVector3D e0 = (p-fc).normalized();
    QVector3D e1 = QVector3D::crossProduct(e2,e0).normalized();
    return QMatrix4x4(
        e0.x(),e1.x(),e2.x(),fc.x(),
        e0.y(),e1.y(),e2.y(),fc.y(),
        e0.z(),e1.z(),e2.z(),fc.z(),
        0,0,0,1);
}


//-------------------------------------------------------------------


QVector3D getCenter(const Polyhedron*ph)
{
  QVector3D center;
  int n = ph->getVertexCount();
  assert(n>=4);
  for(int i=0;i<n;i++)
    center += ph->getVertex(i).m_pos;
  center/=n;
  return center;
}


//-------------------------------------------------------------------

void transform(Polyhedron *ph, const QMatrix4x4 &matrix)
{
  for(int i=0;i<ph->getVertexCount();i++)
    ph->getVertex(i).m_pos = matrix.map(ph->getVertex(i).m_pos);
}


//===================================================================

void Polyhedron::serialize(std::ostream &os)
{
  Polyhedron *ph = this;
  int vCount = ph->getVertexCount();
  int eCount = ph->getEdgeCount();
  int fCount = ph->getFaceCount();
  os << vCount << '\n';
  int i;
  for(i=0;i<vCount;i++)
    {
     QVector3D p = ph->getVertex(i).m_pos;
     os << ' ' << p.x() << ' ' << p.y() << ' ' << p.z() << '\n';
    }
  os << eCount << '\n';
  for(i=0;i<eCount;i++)
    {
     const Polyhedron::Edge &e = ph->getEdge(i);
     os << ' ' << e.m_a << ' ' << e.m_b << '\n';
    }
  os << fCount << '\n';
  for(i=0;i<fCount;i++)
    {
     const Polyhedron::Face &f = ph->getFace(i);
     int m = f.m_edges.size();
     os << ' ' << m;
     for(int j=0;j<m;j++)  
       os << ' ' << f.m_edges[j];
     os << '\n';
    }
}

//-------------------------------------------------------------------

Polyhedron *Polyhedron::deserialize(std::istream &is)
{
  int vCount,eCount,fCount;
  is >> vCount;
  std::vector<Polyhedron::Vertex> vertices;
  vertices.reserve(vCount);
  int i;
  for(i=0;i<vCount;i++)
    {
     double x,y,z;
     is >> x >> y >> z;
     QVector3D p(x,y,z);
     vertices.push_back(Polyhedron::Vertex(p));
    }
  is >> eCount;
  std::vector<Polyhedron::Edge> edges;
  edges.reserve(eCount);
  for(i=0;i<eCount;i++)
    {
     int a,b;
     is >> a >> b;
     edges.push_back(Polyhedron::Edge(a,b));
    }
  is >> fCount;
  std::vector<Polyhedron::Face> faces;
  faces.reserve(fCount);
  for(i=0;i<fCount;i++)
    {
     int m;
     is >> m;
     std::vector<int> face(m);
     for(int j=0;j<m;j++) is>>face[j];
     ;
     faces.push_back(Polyhedron::Face(face));
    }
  return new Polyhedron(vertices,edges,faces);
}


//-------------------------------------------------------------------


//=============================================================================

void addFace(std::vector<int> &f, int a, int b, int c)
{
  f.push_back(3);
  f.push_back(a);
  f.push_back(b);
  f.push_back(c);
}

//-----------------------------------------------------------------------------

void addFace(std::vector<int> &f, int a, int b, int c, int d, int e)
{
  f.push_back(5);
  f.push_back(a);
  f.push_back(b);
  f.push_back(c);
  f.push_back(d);
  f.push_back(e);
}

//-----------------------------------------------------------------------------

void addFace(std::vector<int> &f, const std::vector<int> &ff)
{
  f.push_back(ff.size());
  f.insert(f.end(),ff.begin(), ff.end());
}

//-----------------------------------------------------------------------------

void orientFace(
    const std::vector<QVector3D> &pts,
    Polyhedron::Indices &vv,
    QVector3D &center,
    QVector3D &normal)
{
  center = QVector3D();
  int m = vv.size();
  assert(m>=3);
  int i;
  for(i=0;i<m;i++) center += pts[vv[i]];
  center /= m;
  QVector3D w = QVector3D::crossProduct(pts[vv[1]]-pts[vv[0]], pts[vv[0]]-center);
  normal = w.normalized();
  if(QVector3D::dotProduct(normal,center)<0) 
    { 
     normal = -normal;
     std::reverse(vv.begin(), vv.end());
    }
}

//-----------------------------------------------------------------------------

void getAndNormalizePoints(std::vector<QVector3D> &pts, Polyhedron *ph)
{
  if(!ph->areFaceVerticesValid())
    ph->computeFaceVertices();
  int n = ph->getVertexCount();
  assert(n>=4);
  pts.resize(n);
  pts[0] = ph->getVertex(0).m_pos;
  double factor = 1.0/pts[0].length();
  pts[0] *= factor;
  for(int i=1;i<n;i++)
    pts[i] = factor * ph->getVertex(i).m_pos;
}

//-----------------------------------------------------------------------------

void orientFace(
    const std::vector<QVector3D> &pts,
    Polyhedron::Indices &vv)
{
  QVector3D center,normal;
  orientFace(pts,vv,center,normal);
}

//-----------------------------------------------------------------------------

Polyhedron *makeTetrahedron()
{
  double r = 1.0;
  const double u = r/sqrt(3.0);
  std::vector<QVector3D> pts;
  pts.push_back(QVector3D(u,u,u));
  pts.push_back(QVector3D(-u,-u,u));
  pts.push_back(QVector3D(-u,u,-u));
  pts.push_back(QVector3D(u,-u,-u));
  int ff[] = { 3,2,1,0, 3,3,0,1, 3,3,1,2, 3,3,2,0 };
  std::vector<int> faces(ff,ff+16);
  Polyhedron *ph = makePolyhedron(pts, faces);
  return ph;
}

//-----------------------------------------------------------------------------

Polyhedron *makeEsahedron()
{
  double r = 1.0;
  const double u = r/sqrt(3.0);
  std::vector<QVector3D> pts;
  for(int i=0;i<8;i++)
  {
    double x = (i&1)*2-1;
    double y = ((i>>1)&1)*2-1;
    double z = ((i>>2)&1)*2-1;
    pts.push_back(QVector3D(u*x,u*y,u*z));
  }
  int ff[] = { 4,0,1,3,2, 4,4,6,7,5, 4,1,0,4,5, 4,3,1,5,7, 4,2,3,7,6, 4,0,2,6,4 };
  std::vector<int> faces(ff,ff+30);
  Polyhedron *ph = makePolyhedron(pts, faces);
  return ph;
}

//-----------------------------------------------------------------------------

Polyhedron *makeOctahedron()
{
  double r = 1.0;
  double vv[] = {0,0,r,  r,0,0, 0,r,0, -r,0,0, 0,-r,0, 0,0,-r};
  std::vector<QVector3D> pts;
  for(int i=0;i<18;i+=3) pts.push_back(QVector3D(vv[i],vv[i+1],vv[i+2]));
  int ff[] = { 3,5,1,2, 3,5,2,3, 3,5,3,4, 3,5,4,1, 3,0,2,1, 3,0,3,2, 3,0,4,3, 3,0,1,4};
  std::vector<int> faces(ff,ff+32);
  Polyhedron *ph = makePolyhedron(pts, faces);
  return ph;
}

//-----------------------------------------------------------------------------

Polyhedron *makeDodecahedron_old()
{
  double r = 1.0;

  double vv[] = {
    0.607062, 0, 0.794654,
   0.982247, 0, 0.187592,
   0.187592, 0.57735, 0.794654,
   0.303531, 0.934172, 0.187592,
   -0.491124, 0.356822, 0.794654,
   -0.794655, 0.57735, 0.187592,
   -0.491123, -0.356822, 0.794654,
   -0.794654, -0.577351, 0.187592,
   0.187593, -0.57735, 0.794654,
   0.303532, -0.934172, 0.187592,
   -0.303532, 0.934172, -0.187592,
   -0.187593, 0.57735, -0.794654,
   0.794654, 0.577351, -0.187592,
   0.491123, 0.356822, -0.794654,
   0.794655, -0.57735, -0.187592,
   0.491124, -0.356822, -0.794654,
   -0.303531, -0.934172, -0.187592,
   -0.187592, -0.57735, -0.794654,
   -0.982247, 0, -0.187592,
   -0.607062, 0, -0.794654
  };

  double factor = r / QVector3D(vv[0],vv[1],vv[2]).length();
  std::vector<QVector3D> pts;
  for(int i=0;i<3*20;i+=3) pts.push_back(factor*QVector3D(vv[i],vv[i+1],vv[i+2]));

  int ff[] = { 
    5,  0,2,4,6,8,  
    5,  2,0,1,12,3,  
    5,  0,8,9,14,1, 
    5,  8,6,7,16,9,  
    5,  6,4,5,18,7, 
    5,  4,2,3,10,5,  
    5,  3,12,13,11,10,  
    5,  1,14,15,13,12, 
    5,  9,16,17,15,14,  
    5,  7,18,19,17,16,  
    5,  5,10,11,19,18,  
    5,  11,13,15,17,19
  };

  std::vector<int> faces(ff,ff+6*12);
  Polyhedron *ph = makePolyhedron(pts, faces);
  return ph;
}

Polyhedron *makeDodecahedron()
{
    const double r = 1.0;

    const double f = (1.0+sqrt(5.0))*0.5;
    const double g = 1.0/f;

    double vv[] = {
        -1,-1,-1,
         1,-1,-1,
        -1, 1,-1,
         1, 1,-1,
        -1,-1, 1,
         1,-1, 1,
        -1, 1, 1,
         1, 1, 1,

         0, -f, -g, // 8
         0, -f,  g,
         0,  f, -g,
         0,  f,  g,

        -g,  0, -f, // 12
         g,  0, -f,
        -g,  0,  f,
         g,  0,  f,

        -f, -g,  0, // 16
        -f,  g,  0,
         f, -g,  0,
         f,  g,  0,
    };

    // voglio due facce perpendicolari all'asse z.
    double theta = atan(2.0)*0.5;

    QMatrix4x4 rot; 
    rot.setToIdentity(); 
    rot.rotate(-90.0, 0,0,1); // centro=>vertice sull'asse x
    rot.rotate(180.0 * theta / M_PI, 1,0,0);

    double factor = r / QVector3D(vv[0],vv[1],vv[2]).length();
    std::vector<QVector3D> pts;
    for(int i=0;i<3*20;i+=3)
    {
        QVector3D p(vv[i],vv[i+1],vv[i+2]);
        // pts.push_back(rot.map(p)*factor);
        pts.push_back(p*factor);
    }

    int ff[] = { 
        5,   4,9,5,15,14,
        5,   6,14,15,7,11,
        5,   15,5,18,19,7,
        5,   4,14,6,17,16,
        5,   0,12,13,1,8,
        5,   12,2,10,3,13,
        5,   18,5,9,8,1,
        5,   10,11,7,19,3,
        5,   11,10,2,17,6,
        5,    0,8,9,4,16,
        5,   0,16,17,2,12,
        5,   19,18,1,13,3
    };

    std::vector<int> faces(ff,ff+6*12);
    Polyhedron *ph = makePolyhedron(pts, faces);
    return ph;
}

//-----------------------------------------------------------------------------

Polyhedron *makeIcosahedron()
{
  double r = 1.0;
  double vv[] = {
    -0.525731,-0.850651,0,  
    0.525731,-0.850651,0,  
    -0.525731,0.850651,0,
    0.525731,0.850651,0,  
    -0.850651,0,-0.525731,  
    -0.850651,0,0.525731,  
    0.850651,0,-0.525731,  
    0.850651,0,0.525731,  
    0,-0.525731,-0.850651,  
    0,0.525731,-0.850651,  
    0,-0.525731,0.850651,  
    0,0.525731,0.850651
  };

  double factor = r / QVector3D(vv[0],vv[1],vv[2]).length();
  std::vector<QVector3D> pts;
  for(int i=0;i<3*12;i+=3) pts.push_back(factor*QVector3D(vv[i],vv[i+1],vv[i+2]));

  int ff[] = { 
    3,  3,2,9,  
    3,  6,3,9,  
    3,  8,6,9,  
    3,  4,8,9,  
    3,  2,4,9,  
    3,  1,0,10,  
    3,  0,5,10,  
    3,  5,11,10,  
    3,  10,11,7,  
    3,  7,1,10,  
    3,  2,11,5,  
    3,  3,7,11,  
    3,  1,7,6,  
    3,  0,1,8,  
    3,  5,0,4,  
    3,  2,3,11,  
    3,  3,6,7,  
    3,  8,1,6,  
    3,  4,0,8,  
    3,  2,5,4
  };

  std::vector<int> faces(ff,ff+4*20);
  Polyhedron *ph = makePolyhedron(pts, faces);
  return ph;
}

//-----------------------------------------------------------------------------

Polyhedron *makeDual(const Polyhedron *src)
{
  std::vector<Polyhedron::Vertex> vv;
  std::vector<Polyhedron::Edge> ee;
  std::vector<Polyhedron::Face> ff;
  int i;
  for(i=0;i<src->getFaceCount();i++)
    vv.push_back(Polyhedron::Vertex(getFaceCenter(src, i)));
  std::map<std::pair<int, int>, int> edgeTable;
  std::map<std::pair<int, int>, int>::iterator edgeTableIt;

  for(i=0;i<src->getVertexCount();i++)
  {
    std::vector<int> fv;
    getVertexFaces(fv,src,i);
    int m = (int)fv.size();
    Polyhedron::Indices faceIndices;
    for(int j=0;j<m;j++)
    {
      int iva = fv[j];
      int ivb = fv[(j+1)%m];
      int ei;
      std::pair<int,int> key(iva,ivb);
      edgeTableIt = edgeTable.find(key);
      if(edgeTableIt == edgeTable.end())
      {
        ei = ee.size();
        ee.push_back(Polyhedron::Edge(iva,ivb));
        edgeTable[key] = ei;
        edgeTable[std::make_pair(ivb,iva)] = ei;
      }
      else
        ei = edgeTableIt->second;
      faceIndices.push_back(ei);
    }
    ff.push_back(Polyhedron::Face(faceIndices));
  }
  return new Polyhedron(vv,ee,ff);
}


void getAdjacentVertices(Polyhedron::Indices &indices, const Polyhedron*ph, int vi)
{
    const Polyhedron::Vertex &v = ph->getVertex(vi);
    std::vector<std::pair<int, int> > links;
    QVector3D p0 = v.m_pos;
    for(int fi=0;fi<ph->getFaceCount();fi++)
    {
        QList<int> ab;
        const Polyhedron::Face &f = ph->getFace(fi);
        for(int j=0;j<(int)f.m_edges.size();j++)
        {
            int ei = f.m_edges[j];
            const Polyhedron::Edge &e = ph->getEdge(ei);
            if(e.m_a == vi) ab.append(e.m_b);
            else if(e.m_b == vi) ab.append(e.m_a);
        }
        assert(ab.empty() || ab.count()==2);
        if(!ab.empty())
        {
            int a = ab[0], b = ab[1];
            if(QVector3D::dotProduct(QVector3D::crossProduct(ph->getVertex(b).m_pos-p0, ph->getVertex(a).m_pos-p0), p0)>0) qSwap(a,b);
            links.push_back(std::make_pair(a,b));

            assert(QVector3D::dotProduct(p0, QVector3D::crossProduct(ph->getVertex(b).m_pos-p0, ph->getVertex(a).m_pos-p0))<0);

        }
    }

    unroll(indices, links);

    int m = (int)indices.size();
    for(int j=0;j<m;j++)
    {
        QVector3D p1 = ph->getVertex(indices[j]).m_pos;
        QVector3D p2 = ph->getVertex(indices[(j+1)%m]).m_pos;
        QVector3D u = QVector3D::crossProduct(p2-p0,p1-p0);
        assert(QVector3D::dotProduct(u, p0)>0.0);

    }

}


Polyhedron *rectificate(const Polyhedron *src)
{
    std::vector<QVector3D> pts;
    QMap<QPair<int, int>, int> table; // (a,b) => pts index
    for(int i=0;i<src->getEdgeCount();i++)
    {
        const Polyhedron::Edge &edge = src->getEdge(i);
        int a = edge.m_a;
        int b = edge.m_b;
        QVector3D pa = src->getVertex(a).m_pos;
        QVector3D pb = src->getVertex(b).m_pos;
        int index = (int)pts.size();
        pts.push_back(0.5*(pa+pb));
        table[qMakePair(a,b)] = index;
        table[qMakePair(b,a)] = index;
    }
    std::vector<int> faces;
    // vertex faces
    for(int i=0;i<src->getVertexCount();i++)
    {
        Polyhedron::Indices vv;
        getAdjacentVertices(vv, src, i);
        faces.push_back(vv.size());
        for(int j=0; j<vv.size();j++)
        {
            int k = table[qMakePair(i, vv[j])];
            faces.push_back(k);
        }
    }
    
    // face faces

    for(int i=0;i<src->getFaceCount();i++)
    {
        const Polyhedron::Face &face = src->getFace(i);
        faces.push_back(face.m_edges.size());
        for(int j=0;j<face.m_edges.size();j++)
        {
            const Polyhedron::Edge &edge = src->getEdge(face.m_edges[j]);
            int k = table[qMakePair(edge.m_a, edge.m_b)];
            faces.push_back(k);
        }
    }
    // attenzione! C'� una pazzia: makePolyhedron non rispetta l'orientamento delle facce :(
    return makePolyhedron(pts, faces);
}


Polyhedron *truncate(const Polyhedron *src, double t) 
{
    assert(t<0.5);
    std::vector<QVector3D> pts;
    QMap<QPair<int, int>, int> table; // (a,b) => indice del nuovo punto pi� vicino ad a
    for(int i=0;i<src->getEdgeCount();i++)
    {
        const Polyhedron::Edge &edge = src->getEdge(i);
        int a = edge.m_a;
        int b = edge.m_b;
        QVector3D pa = src->getVertex(a).m_pos;
        QVector3D pb = src->getVertex(b).m_pos;
        
        table[qMakePair(a,b)] = pts.size();
        pts.push_back((1-t)*pa + t*pb);
        table[qMakePair(b,a)] = pts.size();
        pts.push_back((1-t)*pb + t*pa);
    }

    std::vector<int> faces;
    // vertex faces
    for(int i=0;i<src->getVertexCount();i++)
    {
        Polyhedron::Indices vv;
        getAdjacentVertices(vv, src, i);
        faces.push_back(vv.size());
        for(int j=0; j<vv.size();j++)
        {
            int k = table[qMakePair(i, vv[j])];
            faces.push_back(k);
        }
    }
    
    // face faces

    for(int i=0;i<src->getFaceCount();i++)
    {

        Polyhedron::Indices vv;
        getFaceVertices(vv, src, i);
        int m = (int)vv.size();
        faces.push_back(2*m);
        for(int j=0;j<m;j++)
        {
            int a = vv[j], b = vv[(j+1)%m];
            faces.push_back(table[qMakePair(a,b)]);
            faces.push_back(table[qMakePair(b,a)]);
        }
    }
    // attenzione! C'� una pazzia: makePolyhedron non rispetta l'orientamento delle facce :(
    return makePolyhedron(pts, faces);

}



Polyhedron *makeCuboctahedron()
{
    Polyhedron *cube = makeCube();
    //checkOrientation(cube);
    Polyhedron *ph = rectificate(cube);
    delete cube;
    //checkOrientation(ph);
    return ph;
}


Polyhedron *makeTruncatedDodecahedron()
{
    Polyhedron *dod = makeDodecahedron();
    const double t = 0.1*(5-sqrt(5.0));
    Polyhedron *ph = truncate(dod,t);
    delete dod;
    return ph;
}


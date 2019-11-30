#include "FoldingFacesPage.h"

#include <math.h>
#include <assert.h>
#include <QKeyEvent>
#include <QWheelEvent>
#include <qdebug.h>

#include <vector>
#include <stdlib.h>
#include <assert.h>
#include <fstream>
#include <set>

#include "Gutil.h"
#include "Point3.h"

#include "Net.h"

#define PI 3.141592653

using namespace GmLib;


// GLTexture *texture = 0;

Point3 gog0,gog1,gog2, goge0,goge1,goge2;


double rnd()
{
  return 2*((double)rand()/RAND_MAX)-1;
}

class Mesh3D : public Net {  
  int m_type;
  double m_faceSpring;

public:
  class Vertex : public Net::Vertex {
  public:

    Point3 m_pos, m_speed;
    int m_type;
    int m_distance;
    Vertex() : m_type(0), m_distance(-1) {}
  };

  Mesh3D() : Net(), m_faceSpring(0.01) {}
  void setType(int type) {
    if(type<0) type=0;
    else if(type>3)type=3;
    m_type = type;
    clear();
    makeFirstFace();
  }
  int getType() const {return m_type;}

  Net::Vertex *createVertex() { return add(new Vertex()); }
  Vertex *my(Net::Vertex *v) const {return static_cast<Vertex *>(v);}

  Vertex *getMeshVertex(int index) const {return static_cast<Vertex *>(getVertex(index));}

  int addVertex(const Point3 &pos = Point3(), int type = 6) {
    Vertex *v = my(createVertex());
    v->m_pos=pos;
    v->m_speed=Point3(0,0,0);
    v->m_type = type;
    return v->getIndex();
  }

  Point3 getVertexPos(int i) const { return my(getVertex(i))->m_pos;}

  Point3 getVertexNormal(int i) const {
    Point3 p = getVertexPos(i);
    std::vector<Net::Vertex*> vv;
    getAdjacentVertices(vv,i);
    int n = (int)vv.size();
    std::vector<Point3> pts(n);
    for(int j=0;j<n;j++)
      pts[j] = my(vv[j])->m_pos-p;
    Point3 nrm = cross(pts[n-1],pts[0]);
    for(int j=0;j+1<n;j++)
      nrm += cross(pts[i],pts[i+1]);
    return nrm.normalized();
  }

  void build() {
    m_type = 3;
    clear();
    addVertex(Point3(), 7);
    for(int i=0;i<7;i++)
    {
      double phi = 2*PI*i/7;
      Point3 point(cos(phi),sin(phi),0.0);
      addVertex(point, 6);
    }    
    for(int i=0;i<7;i++) createFace(0,i+1,((i+1)%7)+1);
    int borderLength = 7;
    for(int c=0;c<4;c++)
    {
      double r = 2+c;
      int k1 = getVertexCount();
      int k0 = k1 - borderLength;
      std::vector<int> order(borderLength);
      for(int i=0;i<borderLength;i++)
      {
        Vertex *v = getMeshVertex(k0+i);
        order[i] = v->m_type - v->getEdgeCount() - 2;
      }
      int oldk = -1;
      int firstk = -1;
      for(int i=0;i<borderLength;i++)
      {
        int k = getVertexCount();
        int ka = k0+((i+borderLength-1)%borderLength);
        int kb = k0+i;
        int kc = k0+((i+1)%borderLength);
        Vertex *va = getMeshVertex(ka);
        Vertex *vb = getMeshVertex(kb);
        Vertex *vc = getMeshVertex(kc);
        Point3 pa = va->m_pos;
        Point3 pb = vb->m_pos;
        Point3 pc = vc->m_pos;
        Point3 pab = (pa+pb)*0.5;
        Point3 pbc = (pb+pc)*0.5;
        int no = order[i];
        if(no>0)
        {
          int type = 6;
          if(vb->m_type == 6 &&
             va->m_type == 7 && vc->m_type ==7)
            type = 7;
          for(int j=0;j<no;j++)
          {
            double t = (double)(j+1)/(no+1);
            Point3 p = ((1-t)*pab+t*pbc).normalized()*r;
            addVertex(p, type);
            if(oldk>=0 && firstk>=0)
              createFace(k,kb,oldk);
            oldk = k++;
          }
        }
        addVertex(
            pbc.normalized()*r,
            (vb->m_type==7 || vc->m_type==7) ? 6 : 7);

        if(oldk>=0 && firstk>=0)
          createFace(k,kb,oldk);
        createFace(kc,kb,k);
        if(firstk<0) firstk = k;
        oldk = k++;
      }
      for(int k=k1;k<=firstk;k++)
      {
        createFace(k,k0,oldk);
        oldk=k;
      }
      borderLength = getVertexCount()-k1;
    }
  }


  void makeFirstFace() {
    clear();
    const int nn[][2] = {{6,6},{5,5},{5,6},{7,6}};
    int n = nn[m_type][0];
    int m = nn[m_type][1];
    addVertex(Point3(), n);
    for(int i=0;i<n;i++)
    {
      double phi = 2*PI*i/n;
      addVertex(Point3(cos(phi),0,sin(phi)), m);
    }
    for(int i=0;i+1<n;i++) createFace(0,i+1,i+2);
    createFace(0,n,1);
  }




  void growVertex() {
    // trova il bordo
    HEdge *he = 0;
    for(int i=0;i<getHEdgeCount();i++)
    {
      he = getHEdge(i);
      if(he->getLeftFace()==0) break;
    }
    // niente bordo, esci
    if(!he || he->getLeftFace()!=0) return;

    // trova il vertice piu' completo
    std::vector<int> valence(getVertexCount());
    for(int i=0;i<getVertexCount();i++)
    {
      Vertex *v = getMeshVertex(i);
      valence[i] = v->m_type - getAdjacentVertexCount(v);
    }
    HEdge *startHe=he;
    HEdge *selectedHe=he;
    Vertex *selectedVertex = my(selectedHe->getPrevVertex());
    he=he->getNext();
    while(he!=startHe)
    {
      Vertex *vertex = my(he->getPrevVertex());
      if(valence[vertex->getIndex()]<valence[selectedVertex->getIndex()])
      {
        selectedVertex = vertex;
        selectedHe = he;
      }
      he=he->getNext();
    }

    int d = valence[selectedVertex->getIndex()];
    assert(d>=0);

    Vertex*v0 = selectedVertex;
    Vertex*v1 = my(selectedHe->getPrev()->getPrevVertex());
    Vertex*v2 = my(selectedHe->getNextVertex());
    if(d==0)
    {
      // devo solo chiudere la faccia
      createFace(v1->getIndex(),v0->getIndex(),v2->getIndex());
    }
    else
    {
      const int qq[2][4] = {{12,0,11,13},{0,10,12,12}};
      Point3 p0 = v0->m_pos;
      Point3 p1 = v1->m_pos;
      Point3 p2 = v2->m_pos;

      HEdge *he = v0->getHedge();
      if(!he) qDebug() << "ahia1";
      if(he->getPrevVertex() != v0) qDebug() << "ahia2";
      if(he->getLeftFace()==0)
      {
        qDebug() << "empty face********";
        he = he->getTween();
        if(he->getLeftFace()==0)
          qDebug() << "ARGHHHH********";
      }
      Point3 fp0 = my(he->getNextVertex())->m_pos; he=he->getNext();
      Point3 fp1 = my(he->getNextVertex())->m_pos; he=he->getNext();
      Point3 fp2 = my(he->getNextVertex())->m_pos; ;

      Point3 nrm = cross(fp1-fp0,fp2-fp0).normalized();
      // Point3 nrm = cross(p2-p0,p1-p0).normalized();

      gog0 = p0;
      gog1 = p1;
      gog2 = p2;


      Point3 e0 = (p1-p0).normalized(); // p0 -> p1
      Point3 e1 = cross(e0,nrm);

      goge0 = e0;
      goge1 = e1;
      goge2 = nrm;

      //double phi = atan2(dot(p2-p0,e0), dot(p2-p0,e1));
      double phi1 = atan2(dot(p1-p0,e1), dot(p1-p0,e0));
      double phi2 = atan2(dot(p2-p0,e1), dot(p2-p0,e0));
      //qDebug() << "phi1="  << phi1;
      //qDebug() << "phi2="  << phi2;
      if(phi2<phi1) phi2+=2*PI;

      int q = qq[v0->m_type == 6 ? 0 : 1][m_type];
      Vertex *oldV = v1;
      for(int i=0;i<d;i++)
      {
        // double theta = phi * (i+1) / (d+1);
        double t = (double)(i+1)/(d+1);
        double theta = (1-t)*phi1 + t*phi2;

        Point3 p = p0 + cos(theta)*e0+sin(theta)*e1;
        int type = q - oldV->m_type;
        int vi = addVertex(p,type);
        createFace(oldV->getIndex(),v0->getIndex(),vi);
        oldV = my(getVertex(vi));
      }
      createFace(oldV->getIndex(),v0->getIndex(),v2->getIndex());
    }
  }

  void setFaceSpring(double faceSpring) {
    if(faceSpring<0.0)faceSpring = 0;
    m_faceSpring=faceSpring;
    qWarning("face spring = %lf", faceSpring);
  }
  double getFaceSpring() const {return m_faceSpring;}

  void ballify() {
    int n = getVertexCount();
    if(n<5) return;
    Point3 center;
    for(int i=0;i<n;i++)
      center += getMeshVertex(i)->m_pos;
    center *= (1.0/n);
    double radius=0.0;
    for(int i=0;i<n;i++)
      radius = (center-getMeshVertex(i)->m_pos).getNorm();
    radius *= 1.0/n;
    if(radius < 1) radius = 1;
    for(int i=0;i<n;i++)
    {
      Vertex *v = getMeshVertex(i);
      // v->m_speed = 0;
      Point3 p = v->m_pos - center;
      double pp = p.getNorm();
      if(pp>0.0)
      {
        p *= (radius/pp);
      }
      v->m_pos = center + p;
      v->m_pos += 0.01 * Point3(rnd(),rnd(),rnd());
    }
    qDebug() << "finito";
  }

  void shock() {
    for(int i=0;i<getVertexCount();i++)
    {
      Vertex *v = getMeshVertex(i);
      // v->m_speed += 0.3 * Point3(rnd(),rnd(),rnd());
      v->m_pos += 0.5 * Point3(rnd(),rnd(),rnd());
    }
  }

  double computeEdgeEnergy(Edge *e);

  void process();
  void resetPositions();
  void computeDistances();

  bool isComplete(int d);


  Point3 getFaceCenter(int index);
  Point3 getEdgeCenter(int index) {
    HEdge *he = getEdge(index)->getHedge();
    return 0.5*(my(he->getNextVertex())->m_pos + my(he->getPrevVertex())->m_pos);
  }
  std::pair<Point3, Point3> getFaceCenterAndNormal(int index);
  std::pair<Point3, Point3> getVertexPosAndNormal(int index);
  void save();
};

bool Mesh3D::isComplete(int target)
{
  computeDistances();
  HEdge *he = 0;
  for(int i=0;i<getHEdgeCount();i++)
  {
    he = getHEdge(i);
    if(he->getLeftFace()==0)
    {
      int d = my(he->getNextVertex())->m_distance;
      if(d<target)
        return false;
    }

  }
  return true;
}



double Mesh3D::computeEdgeEnergy(Edge *e)
{
  double energy = 0.0;
  HEdge *he0 = e->getHedge(), *he1 = he0->getTween();
  Vertex *v0 = getMeshVertex(he0->getNextVertex()->getIndex());
  Vertex *v1 = getMeshVertex(he1->getNextVertex()->getIndex());
  energy += 100*pow((v0->m_pos-v1->m_pos).getNorm()-1, 2);

  if(he0->getLeftFace() && he0->getRightFace())
  {
    Point3 p0 = v0->m_pos;
    Point3 p1 = v1->m_pos;
    Point3 p2 = getMeshVertex(he0->getNext()->getNextVertex()->getIndex())->m_pos;
    Point3 p3 = getMeshVertex(he1->getNext()->getNextVertex()->getIndex())->m_pos;
    Point3 nrm0 = cross(p3-p0,p1-p0).normalized();
    Point3 nrm1 = cross(p1-p0,p2-p0).normalized();

    energy += 10*pow((1-dot(nrm0,nrm1)),3);
  }
  return energy;
}


void Mesh3D::process()
{
  int n = getVertexCount();
  if(n<=0) return;
  for(int k=0;k<30;k++)
  for(int i=0;i<n;i++)
  {
    Vertex *v = getMeshVertex(i);
    std::vector<Edge*> edges;
    HEdge *he = v->getHedge();
    HEdge *he1 = he;
    do {
      edges.push_back(he->getEdge());
      if(he->getLeftFace()) edges.push_back(he->getNext()->getEdge());
      he = he->getTween()->getNext();
    } while(he != he1);
    double energy0 = 0.0;
    for(int j=0;j<(int)edges.size();j++)
      energy0 += computeEdgeEnergy(edges[j]);
    Point3 point = v->m_pos;
    v->m_pos += 0.01 * Point3(rnd(),rnd(),rnd());
    double energy1 = 0.0;
    for(int j=0;j<(int)edges.size();j++)
      energy1 += computeEdgeEnergy(edges[j]);

    double delta = energy1 - energy0;

    if(delta<0 || ( delta>0 && exp(-delta*100)>(rnd()+1)*0.5))
    {
    }
    else
    {
      v->m_pos = point;
    }
  }
  double energy = 0.0;
  for(int i=0;i<getEdgeCount();i++)
    energy += computeEdgeEnergy(getEdge(i));
  qDebug() << "energy = " << energy;
  Point3 center;

  for(int i=0;i<n;i++) center += getMeshVertex(i)->m_pos;
  center *= (double)(1.0/n);
  for(int i=0;i<n;i++) getMeshVertex(i)->m_pos -= center;
}


void Mesh3D::computeDistances()
{
  for(int i=0;i<getVertexCount();i++)
    getMeshVertex(i)->m_distance = -1;
  getMeshVertex(0)->m_distance  = 0;
  std::set<int> todo;
  todo.insert(0);
  while(!todo.empty())
  {
    int i = *todo.begin();

    int d = getMeshVertex(i)->m_distance;
    todo.erase(todo.begin());
    std::vector<Net::Vertex*> vv;
    getAdjacentVertices(vv,i);
    for(int j=0;j<(int)vv.size();j++)
    {
      Vertex *v = my(vv[j]);
      if(v->m_distance<0 || v->m_distance>d+1)
      {
        v->m_distance = d + 1;
        todo.insert(v->getIndex());
      }
    }
  }
}

void Mesh3D::resetPositions()
{
  return;
  computeDistances();
  for(int i=0;i<getVertexCount();i++)
  {
    Vertex *v = getMeshVertex(i);
    v->m_pos = Point3();
    v->m_speed = Point3();
  }
  std::vector<Net::Vertex*> vv;
  getAdjacentVertices(vv,0);
  for(int i=0;i<(int)vv.size();i++)
  {
    double phi = 2*PI*i/(vv.size());
    my(vv[i])->m_pos = Point3(cos(phi),sin(phi),0);
  }

  for(int d = 1;;d++)
  {
    qDebug() << "dist=" << d;

    bool done = true;
    for(int i=0;i<getVertexCount();i++)
    {
      Vertex *v = getMeshVertex(i);
      if(v->m_distance != d) continue;
      vv.clear();
      getAdjacentVertices(vv,i);
      int m = (int)vv.size();
      qDebug() << "m=" << m;
      std::vector<int> aa;
      int j;
      for(j=0;j<m;j++)
        if(my(vv[j])->m_distance==d)
          aa.push_back(j);

      qDebug() << "aa.size()=" << aa.size();
      if(aa.size() != 2) continue;
      assert(aa.size()==2);
      done = false;
      if(my(vv[(aa[0]+1)%m])->m_distance<d) qSwap(aa[0],aa[1]);
      Point3 p0 = my(vv[aa[0]])->m_pos;
      Point3 p1 = my(vv[aa[1]])->m_pos;

      std::vector<Vertex*> vv2;
      for(j=(aa[0]+1)%m;j!=aa[1];j=(j+1)%m)
        vv2.push_back(my(vv[j]));
      int m2 = (int)vv2.size();

      for(j=0;j<m2;j++)
      {
        assert(vv2[j]->m_distance == d+1);
        double t = (double)(j+1)/(m2+1);
        vv2[j]->m_pos = ((1-t)*p0+t*p1).normalized() * (d+1);
      }
    }
    if(done) break;
  }
}

Point3 Mesh3D::getFaceCenter(int index)
{
  std::vector<Net::Vertex*> vv;
  getFaceVertices(vv, index);
  int m = (int)vv.size();
  Point3 center;
  for(int i=0;i<m;i++)
    center += my(vv[i])->m_pos;
  center *= (1.0/m);
  return center;
}

std::pair<Point3, Point3> Mesh3D::getFaceCenterAndNormal(int index)
{
  std::vector<Net::Vertex*> vv;
  getFaceVertices(vv, index);
  int m = (int)vv.size();
  Point3 center;
  for(int i=0;i<m;i++)
    center += my(vv[i])->m_pos;
  center *= (1.0/m);
  Point3 nrm;
  for(int i=0;i<m;i++)
    nrm += cross(
        my(vv[(i+1)%m])->m_pos-center,
        my(vv[i])->m_pos-center).normalized();
  nrm = nrm.normalized();
  return std::make_pair(center, nrm);
}


std::pair<Point3, Point3> Mesh3D::getVertexPosAndNormal(int index)
{
  std::vector<Net::Vertex*> vv;
  getAdjacentVertices(vv, index);
  int m = (int)vv.size();
  Point3 pos = getMeshVertex(index)->m_pos;
  Point3 nrm;
  for(int i=0;i<m;i++)
    nrm += cross(
        my(vv[i])->m_pos-pos,
        my(vv[(i+1)%m])->m_pos-pos).normalized();
  nrm = nrm.normalized();
  return std::make_pair(pos, nrm);
}


void Mesh3D::save()
{
  std::ofstream os("../hsoccer.inc");
  std::vector<std::pair<Point3, Point3> >
      fv(getFaceCount()), vv(getVertexCount());
  for(int i=0;i<getFaceCount();i++)
    fv[i] = getFaceCenterAndNormal(i);
  for(int i=0;i<getVertexCount();i++)
    vv[i] = getVertexPosAndNormal(i);
  for (int k = 0; k<2; k++)
  {
    os << "mesh {\n";
    for(int i=0;i<getVertexCount();i++)
    {
      std::vector<HEdge*> hes;
      getOutHedges(hes, i);
      int m = (int)hes.size();
      if(m != 6+k) continue;
      for(int j=0;j<m;j++)
      {
        int k0 = hes[j]->getLeftFace()->getIndex();
        int k1 = hes[(j+1)%m]->getLeftFace()->getIndex();
        Point3 p,nrm;
        os << "  smooth_triangle {\n";
        p = vv[i].first; nrm = vv[i].second;
        os << "    <" << p.x() << "," << p.y() << "," << p.z() << ">,\n";
        os << "    <" << nrm.x() << "," << nrm.y() << "," << nrm.z() << ">,\n";
        p = fv[k0].first; nrm = fv[k0].second;
        os << "    <" << p.x() << "," << p.y() << "," << p.z() << ">,\n";
        os << "    <" << nrm.x() << "," << nrm.y() << "," << nrm.z() << ">,\n";
        p = fv[k1].first; nrm = fv[k1].second;
        os << "    <" << p.x() << "," << p.y() << "," << p.z() << ">,\n";
        os << "    <" << nrm.x() << "," << nrm.y() << "," << nrm.z() << ">}\n";
      }
    }
    if(k==0)
      os << "  texture {WhiteFaceTexture}\n";
    else
      os << "  texture {BlackFaceTexture}\n";
    os << "}\n";    
  }
  os << "union {\n" ;
  for(int i=0;i<getEdgeCount();i++)
  {
    Face *f0 = getEdge(i)->getHedge()->getLeftFace();
    Face *f1 = getEdge(i)->getHedge()->getRightFace();
    if(f0==0 || f1==0) continue;
    Point3 p0 = fv[f0->getIndex()].first;
    Point3 p1 = fv[f1->getIndex()].first;
    os << "  cylinder {";
    os << "<" << p0.x() << "," << p0.y() << "," << p0.z() << ">,";
    os << "<" << p1.x() << "," << p1.y() << "," << p1.z() << ">,";
    os << "BorderThickness}\n";
  }

  os << "  texture {BorderTexture}\n";
  os << "}\n";
}


//=============================================================================

FoldingFacesPage::FoldingFacesPage()
: m_pp(0,0)
, m_rotating(true)
, m_timerId(0)
, m_mesh(new Mesh3D())
, m_cameraDistance(10)
, m_bumpMapShader(0)
{
  memset(m_viewMatrix,0,sizeof(m_viewMatrix));
  for(int i=0;i<16;i+=5) m_viewMatrix[i] = 1.0;
}

FoldingFacesPage::~FoldingFacesPage()
{
  delete m_mesh;
  delete m_bumpMapShader;
}


void FoldingFacesPage::initializeGL()
{
}

void FoldingFacesPage::start()
{
  glEnable(GL_LIGHTING);
  glEnable(GL_DEPTH_TEST);
  glEnable(GL_LIGHT0);
  // glEnable(GL_LIGHT1);
  glLightModelf(GL_LIGHT_MODEL_TWO_SIDE,1.0);
  glLightModelf(GL_LIGHT_MODEL_LOCAL_VIEWER,1.0);

  GLfloat lcolor[] =  { 0.7f, 0.7f, 0.7f, 1.0f};
  GLfloat lpos[]   = { 5, 7, 10, 1.0f};
  glLightfv(GL_LIGHT0, GL_AMBIENT, lcolor);
  glLightfv(GL_LIGHT0, GL_DIFFUSE, lcolor);
  glLightfv(GL_LIGHT0, GL_SPECULAR, lcolor);
  glLightfv(GL_LIGHT0, GL_POSITION, lpos);

  lpos[0] = -5;
  glLightfv(GL_LIGHT1, GL_AMBIENT, lcolor);
  glLightfv(GL_LIGHT1, GL_DIFFUSE, lcolor);
  glLightfv(GL_LIGHT1, GL_SPECULAR, lcolor);
  glLightfv(GL_LIGHT1, GL_POSITION, lpos);
}

void FoldingFacesPage::resizeGL(int width, int height)
{
    double aspect = (float)width/height;
    glViewport(0,0,width,height);
    glMatrixMode(GL_PROJECTION);
    glLoadIdentity();
    gluPerspective(45, aspect, 1.0, 70.0);
    glMatrixMode(GL_MODELVIEW);
    glLoadIdentity();
}


void FoldingFacesPage::paintGL()
{
  m_mesh->process();

  glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
  drawBackground();

  glPushMatrix();
  glLoadIdentity();
  glTranslated(0,0,-m_cameraDistance);

  glMultMatrixf(m_viewMatrix);

  GLfloat specular[] =  { 0.7f, 0.7f, 0.7f, 1.0f};
  glMaterialfv(GL_FRONT_AND_BACK, GL_SPECULAR, specular);
  glMaterialf(GL_FRONT_AND_BACK, GL_SHININESS, 120.0);

  Mesh3D &mesh = *m_mesh;

  double soccerVisual = m_pp.x();
  if(soccerVisual<0) soccerVisual =0;
  else if(soccerVisual>1) soccerVisual=1;
  if(m_mesh->getType()<2) soccerVisual = 0;

  Color v5Color(0.4,0.35,0.1),v6Color(0.8,0.75,0.1),v7Color(0.8,0.1,0.5);
  double gr = (1-soccerVisual)*0.8 + soccerVisual*0.4;
  Color eColor(gr,gr,gr);
  Color fColor(0.2,0.4,0.6);
  Color blackColor(0.1,0.1,0.1);
  Color whiteColor(0.8,0.8,0.8);

  if(soccerVisual>0.0)
  {
    v5Color = mix(v5Color,blackColor,soccerVisual);
    v7Color = mix(v7Color,blackColor,soccerVisual);
    v6Color = mix(v6Color,whiteColor,soccerVisual);
  }

  int vCount = mesh.getVertexCount();
  int eCount = mesh.getEdgeCount();
  int fCount = mesh.getFaceCount();

  std::vector<Point3> vPts(vCount);
  std::vector<Point3> ePts(eCount);
  std::vector<Point3> fPts(fCount);

  for(int i=0;i<vCount;i++) vPts[i] = mesh.my(mesh.getVertex(i))->m_pos;
  for(int i=0;i<fCount;i++) fPts[i] = mesh.getFaceCenter(i);
  for(int i=0;i<eCount;i++) ePts[i] = mesh.getEdgeCenter(i);
  if(soccerVisual>0.0)
  {
    for(int i=0;i<eCount;i++)
    {
      Net::HEdge *he = mesh.getEdge(i)->getHedge();
      Net::Face *f0 = he->getLeftFace();
      Net::Face *f1 = he->getRightFace();
      if(f0==0 || f1==0) continue;
      Point3 p = 0.5*(fPts[f0->getIndex()]+fPts[f1->getIndex()]);
      ePts[i] = (1-soccerVisual) * ePts[i] + soccerVisual * p;
    }
    for(int i=0;i<vCount;i++)
    {
      Mesh3D::Vertex *v = mesh.getMeshVertex(i);
      Net::HEdge *he = v->getHedge();
      Point3 c = ePts[he->getEdge()->getIndex()];
      int n = 1;
      if(he->getLeftFace()==0) continue;
      he = he->getTween()->getNext();
      while(he != v->getHedge() && he->getLeftFace() != 0)
      {
        c += ePts[he->getEdge()->getIndex()];
        he = he->getTween()->getNext();
        n++;
      }
      if(n==0 || he != v->getHedge()) continue;
      c *= 1.0/(double)n;
      vPts[i] = (1-soccerVisual)*vPts[i] + soccerVisual*c;
    }
  }

  if(soccerVisual<1.0)
  {
    // vertici e lati della mesh originale
    for(int i=0;i<vCount;i++)
    {
      Mesh3D::Vertex*v = mesh.getMeshVertex(i);
      if(v->m_type==5) setColor(v5Color);
      else if(v->m_type==7) setColor(v7Color);
      else setColor(v6Color);
      drawSphere(vPts[i],0.05*(1-soccerVisual),10,10);
      // Point3 p = v->m_pos + 0.15*mesh.getVertexNormal(i);
      // renderText(p.x(),p.y(),p.z(), QString::number(v->m_valence),QFont("Verdana",20,QFont::Bold));
    }
    for(int i=0;i<eCount;i++)
    {
      Net::Edge *e = mesh.getEdge(i);
      Point3 p0 = vPts[e->getV0()->getIndex()];
      Point3 p1 = vPts[e->getV1()->getIndex()];
      Point3 pe = ePts[i];
      double d = (p1-p0).getNorm();
      double err = d-1;
      Color color = eColor;
      if(err>0) color = mix(color, Color(1,0.2,0.2), clamp(5*err));
      else color = mix(color, Color(0.2,1,0.2), clamp(-5*err));
      setColor(color);
      double r = 0.02*(1-soccerVisual);
      drawCylinder(p0,pe,r,11,3);
      drawCylinder(p1,pe,r,11,3);
    }
  }
  if(soccerVisual>0)
  {
    for(int i=0;i<mesh.getFaceCount();i++)
    {
      Net::Face *f = mesh.getFace(i);
      Net::HEdge *hes[3];
      hes[0] = f->getHedge();
      hes[1] = hes[0]->getNext();
      hes[2] = hes[1]->getNext();
      assert(hes[2]->getNext() == hes[0]);
      Point3 pc = fPts[i];
      setColor(eColor);
      drawSphere(pc,0.01*soccerVisual,7,7);
      for(int j=0;j<3;j++)
      {
        int j1 = (j+1)%3;
        Color color = mesh.my(hes[j]->getNextVertex())->m_type != 6 ? blackColor : whiteColor;
        setColor(mix(fColor,color,soccerVisual));
        Point3 pa = vPts[hes[j]->getNextVertex()->getIndex()];
        Point3 pb = ePts[hes[j1]->getEdge()->getIndex()];
        Point3 pd = ePts[hes[j]->getEdge()->getIndex()];
        setColor(eColor);
        drawCylinder(pb,pc,0.01*soccerVisual,7,2);
      }
    }
  }

  /*
  glActiveTexture(GL_TEXTURE0);
  texture->bind();
  if(m_bumpMapShader) {
    m_bumpMapShader->bind();
    m_bumpMapShader->setInt("tex", 0);
    // m_bumpMapShader->setInt("tex", 0);
    // int tgAttr = glGetAttribLocationARB(m_bumpMapShader->, "tg");
  }
  */

  for(int i=0;i<mesh.getFaceCount();i++)
  {
    Net::Face *f = mesh.getFace(i);
    Net::HEdge *hes[3];
    hes[0] = f->getHedge();
    hes[1] = hes[0]->getNext();
    hes[2] = hes[1]->getNext();
    assert(hes[2]->getNext() == hes[0]);
    for(int j=0;j<3;j++)
    {
      int j1 = (j+1)%3;
      Color color = mesh.my(hes[j]->getNextVertex())->m_type != 6 ? blackColor : whiteColor;
      setColor(mix(fColor,color,soccerVisual));
      Point3 pa = vPts[hes[j]->getNextVertex()->getIndex()];
      Point3 pb = ePts[hes[j1]->getEdge()->getIndex()];
      Point3 pc = fPts[i];
      Point3 pd = ePts[hes[j]->getEdge()->getIndex()];
      drawTriangle(pd,pa,pc);
      drawTriangle(pa,pb,pc);
    }
  }
  /*
  if(m_bumpMapShader) m_bumpMapShader->unbind();
  glActiveTexture(GL_TEXTURE0);
  texture->unbind();
  */


  glPopMatrix();
}

void FoldingFacesPage::paintGL2()
{
  m_mesh->process();

  glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
  drawBackground();

  glPushMatrix();
  glLoadIdentity();
  glTranslated(0,0,-m_cameraDistance);

  glMultMatrixf(m_viewMatrix);

  GLfloat specular[] =  { 0.7f, 0.7f, 0.7f, 1.0f};
  glMaterialfv(GL_FRONT_AND_BACK, GL_SPECULAR, specular);
  glMaterialf(GL_FRONT_AND_BACK, GL_SHININESS, 120.0);

  Mesh3D &mesh = *m_mesh;

  double soccerVisual = m_pp.x();
  if(soccerVisual<0) soccerVisual =0;
  else if(soccerVisual>1) soccerVisual=1;
  if(m_mesh->getType()<2) soccerVisual = 0;

  Color v5Color(0.4,0.35,0.1),v6Color(0.8,0.75,0.1),v7Color(0.8,0.1,0.5);
  double gr = (1-soccerVisual)*0.8 + soccerVisual*0.4;
  Color eColor(gr,gr,gr);
  Color fColor(0.2,0.4,0.6);
  Color blackColor(0.1,0.1,0.1);
  Color whiteColor(0.8,0.8,0.8);

  if(soccerVisual>0.0)
  {
    v5Color = mix(v5Color,blackColor,soccerVisual);
    v7Color = mix(v7Color,blackColor,soccerVisual);
    v6Color = mix(v6Color,whiteColor,soccerVisual);
  }

  std::vector<Point3> fPts;
  std::vector<Point3> ePts;

  if(soccerVisual>0.0)
  {
    fPts.resize(mesh.getFaceCount());
    ePts.resize(mesh.getEdgeCount());
    for(int i=0;i<mesh.getFaceCount();i++)
    {
      std::vector<Net::Vertex*> vv;
      mesh.getFaceVertices(vv, i);
      int m = (int)vv.size();
      Point3 center;
      for(int j=0;j<m;j++)
        center += mesh.my(vv[j])->m_pos;
      center *= (1.0/m);
      fPts[i] = center;
    }
    for(int i=0;i<mesh.getEdgeCount();i++)
    {
      Net::Edge *e = mesh.getEdge(i);
      Point3 p0 = mesh.my(e->getV0())->m_pos;
      Point3 p1 = mesh.my(e->getV1())->m_pos;
      Point3 p01 = (p0+p1)*0.5;
      Net::Face *f0 = e->getHedge()->getLeftFace();
      Net::Face *f1 = e->getHedge()->getRightFace();
      if(f0==0 || f1==0)
      {
        ePts[i] = p01;
      }
      else
      {
        Point3 pm = (fPts[f0->getIndex()]+fPts[f1->getIndex()])*0.5;
        ePts[i] = p01*(1-soccerVisual) + pm*soccerVisual;
      }
    }
  }

  if(soccerVisual<1.0)
  {
    // vertici e lati della mesh originale
    for(int i=0;i<mesh.getVertexCount();i++)
    {
      Mesh3D::Vertex*v = mesh.getMeshVertex(i);
      if(v->m_type==5) setColor(v5Color);
      else if(v->m_type==7) setColor(v7Color);
      else setColor(v6Color);
      drawSphere(v->m_pos,0.05*(1-soccerVisual),10,10);
      // Point3 p = v->m_pos + 0.15*mesh.getVertexNormal(i);
      // renderText(p.x(),p.y(),p.z(), QString::number(v->m_valence),QFont("Verdana",20,QFont::Bold));
    }
    for(int i=0;i<mesh.getEdgeCount();i++)
    {
      Point3 p0 = static_cast<Mesh3D::Vertex*>(mesh.getEdge(i)->getV0())->m_pos;
      Point3 p1 = static_cast<Mesh3D::Vertex*>(mesh.getEdge(i)->getV1())->m_pos;
      double d = (p1-p0).getNorm();
      double e = d-1;
      Color color = eColor;
      if(e>0) color = mix(color, Color(1,0.2,0.2), clamp(5*e));
      else color = mix(color, Color(0.2,1,0.2), clamp(-5*e));
      setColor(color);
      drawCylinder(p0,p1,0.02*(1-soccerVisual),11,3);
    }
  }
  /*
  glActiveTexture(GL_TEXTURE0);
  texture->bind();
  if(m_bumpMapShader) {
    m_bumpMapShader->bind();
    m_bumpMapShader->setInt("tex", 0);
    // m_bumpMapShader->setInt("tex", 0);
    // int tgAttr = glGetAttribLocationARB(m_bumpMapShader->, "tg");
  }
  */
  for(int i=0;i<mesh.getFaceCount();i++)
  {
    std::vector<Net::Vertex*> vv;
    mesh.getFaceVertices(vv,i);
    assert(vv.size()==3);
    std::vector<Point3> pts(3);
    for(int j=0;j<3;j++)
    {
      Mesh3D::Vertex*v = static_cast<Mesh3D::Vertex*>(vv[j]);
      pts[j] = v->m_pos;
    }
    Point3 nrm = cross(pts[1]-pts[0],pts[2]-pts[0]).normalized();
    if(m_mesh->getType()<2 || soccerVisual==0.0)
    {
      setColor(fColor);
      glBegin(GL_TRIANGLES);
      glNormal(nrm);
      glTexCoord2d(0,0);
      glVertex(pts[0]);
      glTexCoord2d(1,0);
      glVertex(pts[1]);
      glTexCoord2d(1,1);
      glVertex(pts[2]);
      glEnd();
    }
    else
    {
      Point3 pc = (1.0/3.0)*(pts[0]+pts[1]+pts[2]);
      for(int j=0;j<3;j++)
      {
        Mesh3D::Vertex*v0 = static_cast<Mesh3D::Vertex*>(vv[j]);
        Point3 p0 = pts[j];
        Point3 p1 = pts[(j+1)%3];
        Point3 p2 = pts[(j+2)%3];
        Point3 p01 = 0.5*(p0+p1);
        Point3 p02 = 0.5*(p0+p2);
        setColor(eColor );
        drawCylinder(p01,pc,0.02*soccerVisual,7,2);
        if(soccerVisual>0.5) setColor(1,0,1);
        drawSphere(pc,0.04*soccerVisual,7,7);
        Color color = v0->m_type != 6 ? blackColor : whiteColor;
        setColor(mix(fColor,color,soccerVisual));
        glBegin(GL_POLYGON);
        glNormal(nrm);
        glVertex(p0);
        glVertex(p01);
        glVertex(pc);
        glVertex(p02);
        glEnd();
      }
    }
  }
  /*
  if(m_bumpMapShader) m_bumpMapShader->unbind();
  glActiveTexture(GL_TEXTURE0);
  texture->unbind();
  */

  glPopMatrix();
}


void FoldingFacesPage::mousePressEvent(QMouseEvent *e)
{
  m_lastPos = e->pos();
  m_rotating = e->button() == Qt::RightButton;
}

void FoldingFacesPage::mouseReleaseEvent(QMouseEvent *)
{

}

void FoldingFacesPage::mouseMoveEvent(QMouseEvent *e)
{
  QPoint delta = m_lastPos - e->pos();
  m_lastPos = e->pos();
  if(!m_rotating)
  {
    //m_parameter += 0.01 * delta.x();
    //m_surface->computeMesh(SimpleSurface(m_parameter));
    m_pp += 0.01*QPointF(delta.x(), delta.y());
    if(m_pp.x()<0) m_pp.setX(0);
    else if(m_pp.x()>1) m_pp.setX(1);

  }
  else
  {
    glPushMatrix();
    glLoadIdentity();
    glRotated(delta.x(),0,1,0);
    glRotated(-delta.y(),1,0,0);
    glMultMatrixf(m_viewMatrix);
    glGetFloatv(GL_MODELVIEW_MATRIX, m_viewMatrix);
    glPopMatrix();
  }
}

void FoldingFacesPage::keyPressEvent(QKeyEvent *e)
{
  if(Qt::Key_1 <= e->key() && e->key() <= Qt::Key_4)
  {
    m_mesh->setType(e->key()-Qt::Key_1);
  }
  else if(Qt::Key_0 == e->key())
  {
    m_mesh->setType(0);
    m_mesh->build();
  }
  else if(e->key() == Qt::Key_Plus)
  {
    m_mesh->growVertex();
  }
  else if(e->key() == Qt::Key_Z)
  {
    m_mesh->shock();
    // m_mesh->ballify();
    // m_mesh->setFaceSpring(m_mesh->getFaceSpring()+0.01);
  }
  else  if(e->key() == Qt::Key_X)
  {
    // m_mesh->setFaceSpring(m_mesh->getFaceSpring()-0.01);
  }
  else  if(e->key() == Qt::Key_S)
  {
    m_mesh->save();
  }
  else
    e->ignore();

}

void FoldingFacesPage::wheelEvent(QWheelEvent*e)
{
  m_cameraDistance = clamp(m_cameraDistance - e->delta()*0.01, 1, 30);
}


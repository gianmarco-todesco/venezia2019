#include "CircleLimit3Viewer.h"
#include "CircleLimit3Tessellation.h"
#include "Texture.h"
#include "Mesh2D.h"
#include "Gutil.h"

using namespace GmLib;

CircleLimit3Viewer::CircleLimit3Viewer(bool figureMode)
: HTessellationViewer()
, m_tess(new CircleLimit3Tessellation())
, m_status(1)
, m_maxFaceCount(0)
, m_figureMode(figureMode)
{
  m_tess->init(figureMode ? 10000 : 1000);
  setTexture(Texture::get("images/cl3_fish_y.png"));
  m_fish[0] = Texture::get("images/cl3_fish_y.png");
  m_fish[1] = Texture::get("images/cl3_fish_r.png");
  m_fish[2] = Texture::get("images/cl3_fish_g.png");
  m_fish[3] = Texture::get("images/cl3_fish_c.png");

  setMesh(Mesh2D::getMesh("images/cl3_fish.mesh"));
  setHTess(m_tess);
  if(m_figureMode) { m_maxFaceCount= -1; m_status |= 2; }
}

CircleLimit3Viewer::~CircleLimit3Viewer()
{
  delete m_tess;
  m_tess = 0;
}

bool CircleLimit3Viewer::onKeyDown(int key)
{
  if(Qt::Key_0<=key && key<=Qt::Key_9)
  {
    m_status ^= 1 << (key - Qt::Key_0);
  }
  else if(key == Qt::Key_A)
  {
    if(m_maxFaceCount>0) m_maxFaceCount--;
  }
  else if(key == Qt::Key_S)
  {
    m_maxFaceCount++;
  }
  else if(key == Qt::Key_D)
  {
    if(m_maxFaceCount>=0) m_maxFaceCount= -1;
    else m_maxFaceCount = 1;
  }
  else
    return false;
  return true;
}


void CircleLimit3Viewer::draw()
{
  if(!m_texture || !m_mesh || !m_tess) return;

  const double phi = -2*3.14159265/8;
  const double cs = cos(phi), sn = sin(phi);

  int vCount = m_mesh->getVertexCount();
  static QVector<GLfloat> uvArray(vCount*2);
  static QVector<GLfloat> coordArray(vCount*2);
  for(int k=0;k<m_mesh->getVertexCount();k++)
  {
    QPointF uv = m_mesh->my(m_mesh->getVertex(k))->m_uv;
    uvArray[2*k] = uv.x();
    uvArray[2*k+1] = 1-uv.y();
  }
  glEnableClientState(GL_TEXTURE_COORD_ARRAY);
  glEnableClientState(GL_VERTEX_ARRAY);
  glTexCoordPointer(2, GL_FLOAT, sizeof(GL_FLOAT)*2, &uvArray[0]);

  int fCount = m_mesh->getFaceCount();
  QVector<GLubyte> indices(fCount*3);
  for(int k=0;k<m_mesh->getFaceCount();k++)
  {
    std::vector<Net::Vertex*> vv;
    m_mesh->getFaceVertices(vv, k);
    // assert((int)vv.size()==3);
    for(int j=0;j<3;j++)
      indices[k*3+j] = vv[j]->getIndex();
  }

  for(int color = 0; color<4; color++)
  {
    if((m_status&4) && color!=0) continue;
    double brmax = 1;
    if((m_status&8) && color!=0) brmax = 0.2;

    m_fish[color]->bind();
    int faceCount = m_tess->getFaceCount();
    if(m_maxFaceCount>=0 && faceCount>m_maxFaceCount) faceCount = m_maxFaceCount;
    for(int i=0;i<faceCount;i++)
    {
      // if(i>4 && i<9) continue;
      // se voglio fare vedere tutte le colorazioni possibili
      CircleLimit3Tessellation::Face *face = m_tess->my(m_tess->getFace(i));
      int c0 = face->c0, c1 = face->c1;
      HTransform transform = m_gTransform * face->transform;
      double ctest = (transform * Complex(0,0)).getNorm();
      if(ctest>0.99) continue;
      double br = brmax;
      if(ctest>0.97) br = brmax * (1 - 0.5*(ctest-0.97)/0.03);
      //glColor3d(br,br,br);
      glColor3d(1,1,1);

      double cs2=0,sn2=0;

      for(double sgn= -1;sgn<=1;sgn+=2)
      {
        if(c0 == color) { cs2=sn*sgn; sn2=cs*sgn; }
        else if(c1 == color) { cs2= cs*sgn; sn2= -sn*sgn; }
        for(int k=0;k<vCount;k++)
        {
          QPointF p = m_mesh->my(m_mesh->getVertex(k))->m_pos;
          Complex c(cs2*p.x()-sn2*p.y(), sn2*p.x()+cs2*p.y());
          c = transform * c;
          p = QPointF(c.re,c.im) * m_scale + m_pan;
          coordArray[2*k] = p.x();
          coordArray[2*k+1] = p.y();
        }

        glVertexPointer(2, GL_FLOAT, sizeof(GL_FLOAT)*2, &coordArray[0]);
        glDrawElements(GL_TRIANGLES, indices.size(), GL_UNSIGNED_BYTE, &indices[0]);
      }
    }
    m_fish[color]->unbind();
  }
  glDisableClientState(GL_TEXTURE_COORD_ARRAY);
  glDisableClientState(GL_VERTEX_ARRAY);


  glLineWidth(6.0);
  glColor3d(0.1,0.2,0.2);
  drawCircle(m_pan, m_scale, 200);
  glLineWidth(1.0);

  // qDebug() << m_pan.x() << "," << m_pan.y() << "," << m_scale;

  if(m_status&2) drawTess();
}


void CircleLimit3Viewer::drawTess()
{
    if(m_figureMode)
        glColor3d(0,0,0);
    else
        glColor3d(1,0,1);


  for(int i=0;i<m_tess->getEdgeCount();i++)
  {
    Net::HEdge *he = m_tess->getEdge(i)->getHedge();
    Net::Face *f = he->getLeftFace(); if(f==0) f = he->getRightFace();
    HTransform transform = m_gTransform * m_tess->getFaceTransform(f->getIndex());
    int indexInFace = m_tess->my(he)->indexInFace;

    Complex c = transform * m_tess->border(indexInFace, 0);
    QPointF p0 = QPointF(c.re,c.im) * m_scale + m_pan;
    c = transform * m_tess->border(indexInFace, 1);
    QPointF p1 = QPointF(c.re,c.im) * m_scale + m_pan;

    double cordLength = getNorm2(p1-p0);

    int m = 2 + (int)(cordLength*0.1);
    std::vector<std::pair<QPointF, double> > pts(m);
    for(int j=0;j<m;j++)
    {
      Complex c = transform * m_tess->border(indexInFace, (double)j/(double)(m-1));
      double thickness = 4*(1-c.getNorm2());
      pts[j].first = QPointF(c.re,c.im) * m_scale + m_pan;
      pts[j].second = thickness;
    }
    glBegin(GL_LINE_STRIP);
    for(int j=0;j<m;j++)
    {
      QPointF p = pts[j].first;
      QPointF d = pts[j].second * rotate90(normalize(pts[j<m-1?j+1:j].first - pts[j>0?j-1:j].first));
      glVertex(p+d);
      glVertex(p-d);
    }
    glEnd();
  }

}

void CircleLimit3Viewer::foo()
{

  double phi = 0;
  int k = 0;
  CircleLimit3Tessellation::Face *face = 0;
  face = m_tess->my(m_tess->getFace(0));
  double minD2 = (m_gTransform * face->transform * Complex(0,0)).getNorm2();
  int id0 = face->c0*100+face->c1, id1 = face->c1*100 + face->c0;
  for(int i=1;i<m_tess->getFaceCount();i++)
  {
    face = m_tess->my(m_tess->getFace(i));
    int id = face->c0*100 + face->c1;
    if((id-id0)*(id-id1)!=0) continue;
    double d2 = (m_gTransform * m_tess->getFaceTransform(i) * Complex(0,0)).getNorm2();
    if(d2<minD2) {minD2 = d2; k = i; if(id!=id0) phi = 3.1415297 * 0.5;}
  }
  if(k!=0)
  {
    m_gTransform = m_gTransform * m_tess->getFaceTransform(k) * HTransform::rotation(phi) * m_tess->getFaceTransform(0).inverse();
  }


  Complex rPart = m_gTransform.m_a;
  Complex tPart = m_gTransform.m_b / rPart;
  rPart = rPart.normalized();
  if(tPart.getNorm2()>1) tPart = tPart.normalized();

  m_gTransform = HTransform(rPart,rPart*tPart,tPart.conj());

}

void CircleLimit3Viewer::loadTextures()
{
  for(int i=0;i<4;i++)
    m_fish[i]->load();
}

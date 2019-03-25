#include "HTessellationViewer.h"
#include "HTessellation.h"
#include "Mesh2D.h"
#include "Texture.h"
#include "Complex.h"
#include <assert.h>

using namespace GmLib;

HTessellationViewer::HTessellationViewer()
: m_mesh(0)
, m_tess(0)
, m_texture(0)
, m_pan(300,300)
, m_scale(300)
{
}


HTessellationViewer::~HTessellationViewer()
{
}


void HTessellationViewer::setMesh(Mesh2D *mesh)
{
  m_mesh = mesh;
}

void HTessellationViewer::setHTess(HTessellation *tess)
{
  m_tess = tess;
}

void HTessellationViewer::setTexture(Texture *texture)
{
  m_texture = texture;
}

void HTessellationViewer::facePartVertices(const HTransform &tr, double cs, double sn)
{
  assert(m_mesh);
  assert(m_tess);
  // Complex f0 = m_tess->border(0,0), f1 = m_tess->border(2,0);

  for(int k=0;k<m_mesh->getFaceCount();k++)
  {
    std::vector<Net::Vertex*> vv;
    m_mesh->getFaceVertices(vv, k);
    assert((int)vv.size()==3);
    for(int h=0;h<3;h++)
    {
      QPointF p = m_mesh->my(vv[h])->m_pos;
      QPointF uv = m_mesh->my(vv[h])->m_uv;
      Complex c(cs*p.x()-sn*p.y(), sn*p.x()+cs*p.y());
      c = tr * c;
      p = QPointF(c.re,c.im) * m_scale + m_pan;
      glTexCoord2f(uv.x(), 1-uv.y());
      glVertex2d(p.x(), p.y());
    }
  }
}



void HTessellationViewer::draw()
{
  if(!m_texture || !m_mesh || !m_tess) return;

  m_texture->bind();
  glColor3d(1,1,1);
  glBegin(GL_TRIANGLES);

  for(int i=0;i<m_tess->getFaceCount();i++)
  {
    HTransform transform = m_gTransform * m_tess->my(m_tess->getFace(i))->transform;
    double cs = 1, sn = 0;
    for(int j=0;j<4;j++)
    {
      facePartVertices(transform, cs, sn);
      double tmp = cs; cs = -sn; sn = tmp;
    }
  }
  glEnd();
  m_texture->unbind();

  /*
  for(int i=0;i<10; i++) // m_ht->getFaceCount();i++)
  {
  }
  */
}

void HTessellationViewer::drawFaceBorder(int faceIndex)
{
  HTransform transform = m_gTransform * m_tess->getFaceTransform(faceIndex);
  for(int j=0;j<m_tess->getN();j++)
  {
    glBegin(GL_LINE_STRIP);
    int m = 50;
    for(int k=0;k<m;k++)
    {
      Complex c = transform * m_tess->border(j,(double)k/(double)(m-1));
      QPointF p = QPointF(c.re,c.im) * m_scale + m_pan;
      glVertex2d(p.x(),p.y());
    }
    glEnd();
  }
}

void HTessellationViewer::drawAllFacesBorder()
{
  for(int i=0;i<m_tess->getFaceCount();i++)
    drawFaceBorder(i);
}



QPointF HTessellationViewer::getVertexPos(int faceIndex, int vertexIndexInFace) const
{
  HTransform transform = m_gTransform * m_tess->my(m_tess->getFace(faceIndex))->transform;
  Complex c = transform * m_tess->border(vertexIndexInFace,0);
  return QPointF(c.re,c.im) * m_scale + m_pan;
}

QPointF HTessellationViewer::getFaceCenter(int faceIndex) const
{
  HTransform transform = m_gTransform * m_tess->my(m_tess->getFace(faceIndex))->transform;
  Complex c = transform * Complex(0,0);
  return QPointF(c.re,c.im) * m_scale + m_pan;
}

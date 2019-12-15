#ifndef HTESSELLATIONVIEWER_H
#define HTESSELLATIONVIEWER_H

#include "HTransform.h"
#include <QPointF>

class Mesh2D;
class Texture;
namespace GmLib {
  class HTessellation;
  class Complex;
}

class HTessellationViewer
{
protected:
  Mesh2D *m_mesh;
  GmLib::HTessellation *m_tess;
  Texture *m_texture;
  GmLib::HTransform m_gTransform;
  QPointF m_pan;
  float m_scale;

public:
  HTessellationViewer();
  virtual ~HTessellationViewer();

  QPointF getPan() const { return m_pan; }
  float getScale() const { return m_scale; }

  void setMesh(Mesh2D *mesh); // doesn't get ownership
  Mesh2D *getMesh() const { return m_mesh; }

  void setHTess(GmLib::HTessellation *tess); // doesn't get ownership
  GmLib::HTessellation *getHTess() const { return m_tess; }

  void setTexture(Texture *texture); // doesn't get ownership
  Texture *getTexture() const { return m_texture; }

  QPointF getVertexPos(int faceIndex, int vertexIndexInFace) const;
  QPointF getFaceCenter(int faceIndex) const;

  const GmLib::HTransform &getTransform() const {return m_gTransform;}
  void setTransform(const GmLib::HTransform &transform) { m_gTransform = transform; }
  void transform(const GmLib::HTransform &transform) { m_gTransform = transform * m_gTransform;}

  virtual void foo() {}

  virtual void facePartVertices(const GmLib::HTransform &tr, double cs, double sn);
  virtual void draw();
  virtual void drawFaceBorder(int faceIndex);
  virtual void drawAllFacesBorder();

  virtual void resize(int w, int h, double r) {m_pan = QPointF(w*0.5,h*0.5); m_scale = r; }
};

#endif // HTESSELLATIONVIEWER_H

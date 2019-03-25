#ifndef MESH2D_H
#define MESH2D_H

#include <QPointF>
#include <QString>

#include "Net.h"

class Mesh2D : public GmLib::Net {
  QString m_fileName;
public:
  class Vertex : public Net::Vertex {
  public:
    QPointF m_pos, m_uv;
  };

  Mesh2D();

  Net::Vertex *createVertex() { return add(new Vertex()); }
  Vertex *my(Net::Vertex *v) const {return static_cast<Vertex *>(v);}

  Vertex *getMeshVertex(int index) const {return static_cast<Vertex *>(getVertex(index));}

  int addMeshVertex(QPointF pos, const QPointF uv = QPointF()) {
    Vertex *v = my(createVertex());
    v->m_pos=pos;
    v->m_uv=uv;
    return v->getIndex();
  }

  void save(const QString &fileName);
  void save() { if(m_fileName != "") save(m_fileName); }
  void load(const QString &fileName);

  static Mesh2D *getMesh(const QString &filename);

};


#endif // MESH2D_H

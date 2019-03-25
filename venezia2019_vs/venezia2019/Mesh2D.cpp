#include "Mesh2D.h"
#include <fstream>
#include <map>
#include <QFile>
#include <QTextStream>
#include <assert.h>

Mesh2D::Mesh2D()
: m_fileName("")
{
}


void Mesh2D::save(const QString &fileName)
{
  m_fileName = fileName;
  QFile file(fileName);
  if (!file.open(QIODevice::WriteOnly | QIODevice::Text))
    return;
  QTextStream out(&file);
  for(int i=0;i<getVertexCount();i++)
  {
    Mesh2D::Vertex *v = getMeshVertex(i);
    out << 'p' << ' ' << v->m_pos.x() << ' ' << v->m_pos.y() << ' ' << v->m_uv.x() << ' ' << v->m_uv.y() << '\n';
  }
  for(int i=0;i<getFaceCount();i++)
  {
    std::vector<Net::Vertex*> vv;
    getFaceVertices(vv, i);
    assert(vv.size()==3);
    out << 'f';
    for(int j=0;j<3;j++) out << ' ' << vv[j]->getIndex();
    out << '\n';
  }
}

void Mesh2D::load(const QString &fileName)
{
  m_fileName = fileName;
;
  QFile file(fileName);
  if (!file.open(QIODevice::ReadOnly | QIODevice::Text))
    return;
  QTextStream in(&file);
  while (!in.atEnd()) {
    char c;
    in >> c;
    if(c == 'p')
    {
      float x,y,u,v;
      in >> x >> y >> u >> v;
      addMeshVertex(QPointF(x,y), QPointF(u,v));
    }
    else if(c == 'f')
    {
      int a,b,c;
      in >> a >> b >> c;
      createFace(a,b,c);
    }
  }
}

Mesh2D *Mesh2D::getMesh(const QString &filename)
{
  static std::map<QString, Mesh2D*> meshes;
  std::map<QString, Mesh2D*>::iterator it;
  it = meshes.find(filename);
  if(it != meshes.end()) return it->second;
  Mesh2D *mesh = new Mesh2D();
  mesh->load(filename);
  meshes[filename] = mesh;
  return mesh;
}

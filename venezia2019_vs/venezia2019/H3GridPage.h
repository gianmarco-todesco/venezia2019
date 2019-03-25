#ifndef H3GRID_PAGE_INCLUDED
#define H3GRID_PAGE_INCLUDED

#include "OpenGLPage.h"
#include "Mesh.h"
#include <QPointF>
#include <QMatrix4x4>
#include <QGLBuffer>
#include <QTime>
#include "H3Grid.h"


class QGLShaderProgram;
class Texture;
class H3Grid534;


class H3GridPage : public OpenGLPage
{
  double m_theta, m_phi;
  double m_cameraDistance;
  QPoint m_lastPos;
  bool m_rotating;
  QGLShaderProgram *m_shaderProgram;
  QMatrix4x4 m_hMatrix;
  QVector3D m_hOffset;
  H3Grid534 *m_grid;
  Mesh m_sphere, m_vertexCube, m_edgeBox;
  QTime m_clock;

  QList<QMatrix4x4> m_edgeMatrices, m_vertexMatrices;
  int m_level;

  GridMatrices m_gridMatrices;


public:
  H3GridPage();
 ~H3GridPage();

protected:
  void initializeGL();
  void paintGL();
  void resizeGL(int width, int height);

  void makeEdgeBox();
  
  void draw1();
  void draw2();

  void drawVertices();
  void build();

  void drawDodWf(const QMatrix4x4 &mat);
  void drawBoxWf(const QMatrix4x4 &mat, double dx, double dy, double dz);

  void drawEdgeWf(const QMatrix4x4 &mat, double d = 0.05);

  void draw(const QMatrix4x4 &mat, const Mesh &mesh);
  void draw(const QMatrix4x4 &globalMatrix, const GridMatrices &matrices);

  void test();

  void mousePressEvent(QMouseEvent *e);
  void mouseReleaseEvent(QMouseEvent *e);
  void mouseMoveEvent(QMouseEvent *e);

  void keyPressEvent(QKeyEvent *e);
  void wheelEvent(QWheelEvent*);

  void showEvent(QShowEvent*);
  void hideEvent(QHideEvent*);


};

#endif

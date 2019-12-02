#pragma once


#include "Page.h"

#include <QString>
#include <QMatrix4x4>

#include "Mesh.h"
#include <QPointF>
#include <QMatrix4x4>
#include <QGLBuffer>
#include <QTime>
#include "H3Grid.h"


class QGLShaderProgram;
class Texture;
class H3Grid534;
class H3Grid;


class Fig12Page : public Page
{
  double m_theta, m_phi;
  double m_cameraDistance;
  QPoint m_lastPos;
  bool m_rotating;
  QGLShaderProgram *m_shaderProgram;
  QMatrix4x4 m_hMatrix;
  QVector3D m_hOffset;
  H3Grid534 *m_grid;
  H3Grid *m_grid2;
  Mesh m_sphere, m_vertexCube, m_edgeBox, m_edgeBoxLow;
  QTime m_clock;

  QList<QMatrix4x4> m_edgeMatrices, m_vertexMatrices;
  int m_level;

  GridMatrices m_gridMatrices;

  // QList<Uffa> uffa;
  
public:
  Fig12Page();
 ~Fig12Page();

protected:
  void initializeGL();
  void paintGL();
  void resizeGL(int width, int height);

  void makeEdgeBox(Mesh &mesh, int n);
  
  void draw1();
  void draw2();
  void draw3();

  void drawVertices();
  
  void drawDodWf(const QMatrix4x4 &mat);
  void drawBoxWf(const QMatrix4x4 &mat, double dx, double dy, double dz);

  void drawEdgeWf(const QMatrix4x4 &mat, double d = 0.05);

  void draw(const QMatrix4x4 &mat, const Mesh &mesh);
  
  void drawChannels();

  void test3();
  void buildGrid();


  void mousePressEvent(QMouseEvent *e);
  void mouseReleaseEvent(QMouseEvent *e);
  void mouseMoveEvent(QMouseEvent *e);

  void keyPressEvent(QKeyEvent *e);
  void wheelEvent(QWheelEvent*);

  void uffauffa();

  void savePictures();
};


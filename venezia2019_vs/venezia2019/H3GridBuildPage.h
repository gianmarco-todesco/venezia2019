#ifndef H3GRID_BUILD_PAGE_INCLUDED
#define H3GRID_BUILD_PAGE_INCLUDED

#include "Page.h"
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
class Polyhedron;



class H3GridBuildPage : public Page
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
  int m_status;
  int m_statusStartTime;
  int m_panMode;
  double m_panParameter;
  int m_uffa;
  int m_uffa2;
  
public:
  H3GridBuildPage();
 ~H3GridBuildPage();

protected:
    void start();
  void initializeGL();
  void paintGL();
  void resizeGL(int width, int height);

  void makeEdgeBox();
  
  void draw2();
  void draw3();
  void draw4();
  void draw4bis();


  void draw(const QMatrix4x4 &mat, const Mesh &mesh);

  void mousePressEvent(QMouseEvent *e);
  void mouseReleaseEvent(QMouseEvent *e);
  void mouseMoveEvent(QMouseEvent *e);

  void keyPressEvent(QKeyEvent *e);
  void wheelEvent(QWheelEvent*);

  void drawHLine(const QVector3D &p0, const QVector3D &p1);
  void drawHPolyhedron(Polyhedron *ph, const QMatrix4x4 &matrix);
};

#endif

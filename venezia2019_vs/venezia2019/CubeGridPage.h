#ifndef CUBE_GRID_PAGE_H
#define CUBE_GRID_PAGE_H

#include "OpenGLPage.h"
#include "Mesh.h"
#include <QPointF>

class QGLShaderProgram;
class Texture;
class Polydron;

class CubeGridPage : public OpenGLPage
{
  double m_cameraDistance;
  double m_theta, m_phi;
  QPoint m_lastPos;
  bool m_rotating;
  Mesh m_mesh;

public:
  CubeGridPage();
 ~CubeGridPage();


protected:
  void initializeGL();
  void paintGL();
  void resizeGL(int width, int height);

  void draw();

  void mousePressEvent(QMouseEvent *e);
  void mouseReleaseEvent(QMouseEvent *e);
  void mouseMoveEvent(QMouseEvent *e);

  void keyPressEvent(QKeyEvent *e);
  void wheelEvent(QWheelEvent*);

  void showEvent(QShowEvent*);
  void hideEvent(QHideEvent*);

private:
  void buildMesh();

};


#endif // ROLLINGSPHEREPAGE_H

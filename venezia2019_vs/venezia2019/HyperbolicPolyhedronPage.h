#ifndef HYPERBOLIC_POLYHEDRON_PAGE_H
#define HYPERBOLIC_POLYHEDRON_PAGE_H


#include "Page.h"
#include "Mesh.h"
#include "Texture.h"
#include <QPointF>
#include <QTime>

class QGLShaderProgram;
class Texture;
class Polydron;

class HyperbolicPolyhedronPage : public Page
{
  double m_cameraDistance;
  double m_theta, m_phi;
  QPoint m_lastPos;
  bool m_rotating;
  QTime m_clock;

  MyTexture m_texture1;
  Mesh m_mesh, m_hlineMesh;
  Mesh m_outSphereMesh;
  QGLShaderProgram *m_h3ShaderProgram;
  QGLShaderProgram *m_stdShaderProgram;

  double m_parameter;

public:
  HyperbolicPolyhedronPage();
 ~HyperbolicPolyhedronPage();

protected:
    void start();
    void initializeGL();
    void paintGL();
    void resizeGL(int width, int height);

    void createHLineMesh();

    void draw();
    void drawOutSphere();

    void mousePressEvent(QMouseEvent *e);
    void mouseReleaseEvent(QMouseEvent *e);
    void mouseMoveEvent(QMouseEvent *e);

    void keyPressEvent(QKeyEvent *e);
    void wheelEvent(QWheelEvent*);
};


#endif
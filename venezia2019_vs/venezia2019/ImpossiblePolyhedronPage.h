#ifndef IMPOSSIBLE_POLYHEDRON_PAGE_H
#define IMPOSSIBLE_POLYHEDRON_PAGE_H


#include "Page.h"
#include "Mesh.h"
#include <QPointF>
#include <QTime>

class QGLShaderProgram;
class Texture;
class Polydron;

class ImpossiblePolyhedronPage : public Page
{
  double m_cameraDistance;
  double m_theta, m_phi;
  QPoint m_lastPos;
  bool m_rotating;
  QTime m_clock;
  double m_parameter;


public:
  ImpossiblePolyhedronPage();
 ~ImpossiblePolyhedronPage();

protected:
    void start();
    void initializeGL();
    void paintGL();
    void resizeGL(int width, int height);

    void draw();

    void mousePressEvent(QMouseEvent *e);
    void mouseReleaseEvent(QMouseEvent *e);
    void mouseMoveEvent(QMouseEvent *e);

    void keyPressEvent(QKeyEvent *e);
    void wheelEvent(QWheelEvent*);
};


#endif


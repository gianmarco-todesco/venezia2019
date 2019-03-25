#ifndef POLYDRON_PAGE_H
#define POLYDRON_PAGE_H

#include "OpenGLPage.h"
#include "Mesh.h"
#include <QPointF>

class QGLShaderProgram;
class Texture;
class Polydron;

class PolydronPage : public OpenGLPage
{
  double m_cameraDistance;
  double m_theta, m_phi;
  QPoint m_lastPos;
  bool m_rotating;
  Polydron *m_polydron;

public:
  PolydronPage();
 ~PolydronPage();

protected:
  void initializeGL();
  void paintGL();
  void resizeGL(int width, int height);

  void createPieces();
  void draw();

  void mousePressEvent(QMouseEvent *e);
  void mouseReleaseEvent(QMouseEvent *e);
  void mouseMoveEvent(QMouseEvent *e);

  void keyPressEvent(QKeyEvent *e);
  void wheelEvent(QWheelEvent*);

  void showEvent(QShowEvent*);
  void hideEvent(QHideEvent*);
};


#endif // ROLLINGSPHEREPAGE_H

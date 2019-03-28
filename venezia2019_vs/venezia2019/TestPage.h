#ifndef TESTPAGE_H
#define TESTPAGE_H

#include "Page.h"
#include <QPointF>

class QGLShaderProgram;
class Texture;

class TestPage : public Page
{
  double m_theta, m_phi;
  QPointF m_pp;
  QPoint m_lastPos;
  bool m_rotating;
  QGLShaderProgram *m_shader0, *m_shaderBG;
  Texture *m_texture;
  double m_cameraDistance;
  int m_timerId;

  double m_scaleFactor;

public:
  TestPage();
 ~TestPage();

protected:
  void initializeGL();
  void paintGL();
  void resizeGL(int width, int height);

  void drawBoundary();

  void mousePressEvent(QMouseEvent *e);
  void mouseReleaseEvent(QMouseEvent *e);
  void mouseMoveEvent(QMouseEvent *e);

  void keyPressEvent(QKeyEvent *e);
  void wheelEvent(QWheelEvent*);

  
};


#endif // ROLLINGSPHEREPAGE_H

#ifndef FOLDINGFACESPAGE_H
#define FOLDINGFACESPAGE_H

#include "Page.h"
#include "Gutil.h"
#include <QPointF>

class Mesh3D;
class QGLShaderProgram;

class FoldingFacesPage : public Page
{
  QPointF m_pp;
  QPoint m_lastPos;
  bool m_rotating;
  int m_timerId;
  Mesh3D *m_mesh;
  GLfloat m_viewMatrix[16];
  double m_cameraDistance;
  QGLShaderProgram *m_bumpMapShader;

public:
  FoldingFacesPage();
 ~FoldingFacesPage();


  void drawSurface();

protected:
  void initializeGL();
  void paintGL();
  void paintGL2();
  void resizeGL(int width, int height);

  void mousePressEvent(QMouseEvent *e);
  void mouseReleaseEvent(QMouseEvent *e);
  void mouseMoveEvent(QMouseEvent *e);

  void keyPressEvent(QKeyEvent *e);
  void wheelEvent(QWheelEvent*);

  void timerEvent(QTimerEvent*);
  // void showEvent(QShowEvent*);
  void hideEvent(QHideEvent*);

};

#endif // FOLDINGFACESPAGE_H

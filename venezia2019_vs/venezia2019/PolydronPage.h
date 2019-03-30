#ifndef POLYDRON_PAGE_H
#define POLYDRON_PAGE_H

#include "Page.h"
#include "Mesh.h"
#include <QPointF>
#include <QTime>
#include "Texture.h"

class QGLShaderProgram;
class Texture;
class Polydron;

class PolydronPage : public Page
{
  double m_cameraDistance;
  double m_theta, m_phi;
  QPoint m_lastPos;
  bool m_rotating;
  Polydron *m_polydron;
  QTime m_clock;
  double m_parameter; // debug only
  MyTexture m_texture1, m_texture2;

public:
  PolydronPage();
 ~PolydronPage();

protected:
    void start();
    void initializeGL();
    void paintGL();
    void resizeGL(int width, int height);

    void createPieces();
    void draw();
    void drawPolyhedron(); // debug only

    void mousePressEvent(QMouseEvent *e);
    void mouseReleaseEvent(QMouseEvent *e);
    void mouseMoveEvent(QMouseEvent *e);

    void keyPressEvent(QKeyEvent *e);
    void wheelEvent(QWheelEvent*);
};


#endif // ROLLINGSPHEREPAGE_H

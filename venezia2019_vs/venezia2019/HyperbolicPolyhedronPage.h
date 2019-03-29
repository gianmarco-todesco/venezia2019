#ifndef HYPERBOLIC_POLYHEDRON_PAGE_H
#define HYPERBOLIC_POLYHEDRON_PAGE_H


#include "Page.h"
#include "Mesh.h"
#include "Texture.h"
#include <QPointF>
#include <QTime>

class QGLShaderProgram;
class Texture;
class Polyhedron;

class HyperbolicPolyhedronPage : public Page
{
  double m_cameraDistance;
  double m_theta, m_phi;
  QPoint m_lastPos;
  bool m_rotating;
  QTime m_clock;

  MyTexture m_texture1, m_texture2;
  Mesh m_mesh, m_hlineMesh;
  Mesh m_outSphereMesh;
  QGLShaderProgram *m_h3ShaderProgram;
  QGLShaderProgram *m_stdShaderProgram;
  QMatrix4x4 m_hMatrix;

  int m_status;
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

    void drawHPolygon(const QList<QVector3D> &pts);

    void drawHPoint(const QVector3D &pos);
    void drawHLine(const QMatrix4x4 &matrix);
    void drawHLine(const QVector3D &p0, const QVector3D&p1);
    void drawHLines(const QList<QMatrix4x4> &matrices);
    void drawHLines(const QList<QPair<QVector3D, QVector3D> > &pointPairs);
  
    void drawHPolyhedron(const Polyhedron *ph);

    void drawHAngle(const QVector3D &p, const QVector3D &d1, const QVector3D &d2);

    void mousePressEvent(QMouseEvent *e);
    void mouseReleaseEvent(QMouseEvent *e);
    void mouseMoveEvent(QMouseEvent *e);

    void keyPressEvent(QKeyEvent *e);
    void wheelEvent(QWheelEvent*);

    void setStatus(int status);
};


#endif
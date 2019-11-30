#pragma once


#include "Page.h"
#include "Mesh.h"

#include <QString>
#include <QMatrix4x4>

class H3Grid534;
class H3Grid;
class QGLShaderProgram;

class Fig11Page : public Page {
    double m_theta, m_phi;
    double m_cameraDistance;
    H3Grid534 *m_gridData;
    H3Grid *m_grid;
    QGLShaderProgram *m_shaderProgram;
    QMatrix4x4 m_hMatrix;
    Mesh m_edgeBox;
    QPoint m_lastPos;
    bool m_rotating;
  
public:
    Fig11Page();
    ~Fig11Page();

    void buildGrid();
    void makeEdgeBox(Mesh &mesh, int n);

    void initializeGL();
    void resizeGL(int width, int height);
    void paintGL();

    void draw(const QMatrix4x4 &mat, const Mesh &mesh);
  
    void savePictures();
    
protected:
    void mousePressEvent(QMouseEvent *e);
    void mouseReleaseEvent(QMouseEvent *e);
    void mouseMoveEvent(QMouseEvent *e);

    void keyPressEvent(QKeyEvent *e);
    void wheelEvent(QWheelEvent*);

};

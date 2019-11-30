#pragma once


#include "Page.h"

#include <QString>
#include <QMatrix4x4>

class Color;

class Fig5Page : public Page {
    double m_theta, m_phi;
    double m_cameraDistance;
    // QGLShaderProgram *m_shaderProgram;
    QMatrix4x4 m_hMatrix;
    QPoint m_lastPos;
    bool m_rotating;
    double m_parameter;
  
public:
    Fig5Page();
    ~Fig5Page();

    void initializeGL();
    void resizeGL(int width, int height);
    void paintGL();

    
    void savePictures();
    
protected:
    void mousePressEvent(QMouseEvent *e);
    void mouseReleaseEvent(QMouseEvent *e);
    void mouseMoveEvent(QMouseEvent *e);

    void keyPressEvent(QKeyEvent *e);
    void wheelEvent(QWheelEvent*);

    void draw();
    void drawPolygon(int n, double r0, const Color &color);
};

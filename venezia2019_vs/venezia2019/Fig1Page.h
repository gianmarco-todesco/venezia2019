#pragma once


#include "Page.h"

#include <QString>
#include <QMatrix4x4>
#include <QPointF>
#include <QVector3D>
#include "Mesh.h"


class QGLShaderProgram;

class Fig1Page : public Page {
    double m_theta, m_phi;
    double m_cameraDistance;
    // QGLShaderProgram *m_shaderProgram;
    QMatrix4x4 m_hMatrix;
    QPoint m_lastPos;
    bool m_rotating;
    Mesh m_mesh;
    QGLShaderProgram *m_shaderProgram;
    QVector3D m_offset;
    double m_gridSmallUnit;
    double m_gridBigUnit;
    unsigned int m_textureId;

public:
    Fig1Page();
    ~Fig1Page();

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

private:
    void buildMesh();
    void draw();

    void createTextures();

    void makeVertexBox(Mesh &mesh, const QVector3D &pos, double r);
    void makeEdgeBox(Mesh &mesh, const QVector3D &p0, const QVector3D &p1, const QVector3D &up, double r);


    
};

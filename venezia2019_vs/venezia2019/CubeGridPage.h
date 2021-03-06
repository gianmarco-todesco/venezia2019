#ifndef CUBE_GRID_PAGE_H
#define CUBE_GRID_PAGE_H

#include "Page.h"
#include "Mesh.h"
#include "Overlay.h"
#include <QPointF>
#include <QVector3D>

class QGLShaderProgram;
class Texture;
class Polydron;
class OverlayPanel;

class CubeGridPage : public Page
{
    double m_cameraDistance;
    double m_theta, m_phi;
    QPoint m_lastPos;
    bool m_rotating;
    Mesh m_mesh;
    QGLShaderProgram *m_shaderProgram;
    QVector3D m_offset;
    double m_gridSmallUnit;
    double m_gridBigUnit;
    int m_status;
    bool m_translating;
    double m_translatingSpeed;

    struct {
        OverlayPanel title, title2, escher, escherCaption, grids;
    } m_panels;

public:
    CubeGridPage();
    ~CubeGridPage();


protected:
    void start();
    void stop();
    void initializeGL();
    void paintGL();
    void resizeGL(int width, int height);

    void draw();

    void mousePressEvent(QMouseEvent *e);
    void mouseReleaseEvent(QMouseEvent *e);
    void mouseMoveEvent(QMouseEvent *e);

    void keyPressEvent(QKeyEvent *e);
    void wheelEvent(QWheelEvent*);

    void createTextures();
    void destroyTextures();

    void setStatus(int status);

    void savePictures();

private:
    void buildMesh();
    QVector3D getCurrentDirection();
    void moveOffset(const QVector3D &delta);

};


#endif // ROLLINGSPHEREPAGE_H

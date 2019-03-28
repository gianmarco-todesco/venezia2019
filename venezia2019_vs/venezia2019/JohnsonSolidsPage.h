#ifndef JOHNSON_SOLIDS_PAGE_H
#define JOHNSON_SOLIDS_PAGE_H

#include "Page.h"
#include "Mesh.h"
#include "Overlay.h"
#include <QPointF>
#include <QVector3D>

class QGLShaderProgram;
class Texture;
class Polydron;
class OverlayPanel;

class JohnsonSolidsPage : public Page
{
    double m_cameraDistance;
    int m_status;
    struct {
        OverlayPanel wikipage, polyhedra, xyz;
    } m_panels;

public:
    JohnsonSolidsPage();
    ~JohnsonSolidsPage();


protected:
    void start();
    void stop();
    void initializeGL();
    void paintGL();
    void resizeGL(int width, int height);

    void keyPressEvent(QKeyEvent *e);

    void createTextures();
    void destroyTextures();

    void setStatus(int status);

};


#endif // ROLLINGSPHEREPAGE_H

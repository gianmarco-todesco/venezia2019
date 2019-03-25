#ifndef VIEWER_H
#define VIEWER_H

#include <QGLWidget>
#include <QGLContext>
#include <set>
#include <QPointF>
#include <qglbuffer.h>


class QGLShaderProgram;
class Presentation;

class Viewer  : public QGLWidget
{
    Presentation *m_presentation;    

public:
    Viewer();
    ~Viewer();

    void drawAxes(double r = 5.0);
    void drawBackground();
    QGLShaderProgram *loadProgram(QString name);


protected:
    void initializeGL();
    void resizeGL(int width, int height);
    void paintGL();

    QSize sizeHint() const;

    void mousePressEvent(QMouseEvent *e);
    void mouseMoveEvent(QMouseEvent *e);
    void mouseReleaseEvent(QMouseEvent *e);

    void keyPressEvent(QKeyEvent *e);
    void wheelEvent(QWheelEvent*);

    void timerEvent(QTimerEvent*);

private:
    /*
    static std::set<OpenGLPage*> m_sharedSet;
    inline static OpenGLPage* getShared() { return m_sharedSet.empty() ? 0 : *m_sharedSet.begin(); }
    */


};

#endif


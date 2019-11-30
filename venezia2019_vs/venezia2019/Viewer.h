#ifndef VIEWER_H
#define VIEWER_H

#include <QGLWidget>
#include <QGLContext>
#include <set>
#include <QPointF>
#include <qglbuffer.h>
#include <QTime>


class QGLShaderProgram;
class Presentation;
class Overlay;

class Viewer  : public QGLWidget
{
    Presentation *m_presentation;
    Overlay *m_overlay;
    QTime m_clock;
    double m_fps;
    GLuint m_textTextureId;
    bool m_showFps;

public:
    Viewer();
    ~Viewer();

    void drawAxes(double r = 5.0);
    void drawBackground();
    QGLShaderProgram *loadProgram(QString name);

    Overlay *getOverlay() { return m_overlay; }

    void drawText(const QVector3D &pos, const QString &text, double r=1.0, const QColor &color = Qt::black);

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

};

#endif


#ifndef OPENGLPAGE_H
#define OPENGLPAGE_H

#include <QGLWidget>
#include <QGLContext>
#include <set>
#include <QPointF>
#include <qglbuffer.h>


class QGLShaderProgram;

class OpenGLPage  : public QGLWidget
{

public:
    OpenGLPage();
    virtual ~OpenGLPage();



    void drawAxes(double r = 5.0);

protected:
    void initializeGL();
    void resizeGL(int width, int height);
    void drawBackground();

    QGLShaderProgram *loadProgram(QString name);

private:
    static std::set<OpenGLPage*> m_sharedSet;
    inline static OpenGLPage* getShared() { return m_sharedSet.empty() ? 0 : *m_sharedSet.begin(); }


};


#endif // OPENGLPAGE_H

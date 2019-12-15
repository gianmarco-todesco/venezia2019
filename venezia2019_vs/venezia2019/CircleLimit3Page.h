#ifndef CIRCLELIMIT3PAGE_H
#define CIRCLELIMIT3PAGE_H

#include "Page.h"
#include <QTime>
#include <qpoint.h>

class CircleLimit3Viewer;

class CircleLimit3Page : public Page
{
protected:
    CircleLimit3Viewer *m_viewer;
    QPoint m_lastPos;
    QTime m_time;
    int m_timerId;
    int m_status;
    int m_transfType;

public:
    CircleLimit3Page(bool figureMode = false);
    ~CircleLimit3Page();

protected:
    void start();
    void initializeGL();
    void resizeGL(int w, int h);
    void paintGL();

    void mousePressEvent(QMouseEvent *e);
    void mouseReleaseEvent(QMouseEvent *e);
    void mouseMoveEvent(QMouseEvent *e);

    void keyPressEvent(QKeyEvent*);
    void timerEvent(QTimerEvent *);

    void savePage();
};

#endif // CIRCLELIMIT3PAGE_H

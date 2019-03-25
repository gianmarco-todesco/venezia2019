#ifndef CIRCLELIMIT3PAGE_H
#define CIRCLELIMIT3PAGE_H

#include "OpenGLPage.h"
#include <QTime>

class CircleLimit3Viewer;

class CircleLimit3Page : public OpenGLPage
{
  CircleLimit3Viewer *m_viewer;
  QPoint m_lastPos;
  QTime m_time;
  int m_timerId;
  int m_status;
  int m_transfType;

public:
  CircleLimit3Page();
  ~CircleLimit3Page();

protected:
  void paintGL();

  void showEvent(QShowEvent*) {setFocus();}

  void mousePressEvent(QMouseEvent *e);
  void mouseReleaseEvent(QMouseEvent *e);
  void mouseMoveEvent(QMouseEvent *e);

  void keyPressEvent(QKeyEvent*);
  void timerEvent(QTimerEvent *);

  void savePage();
};

#endif // CIRCLELIMIT3PAGE_H

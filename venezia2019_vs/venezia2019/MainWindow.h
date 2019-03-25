#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QStackedWidget>


class MainWindow : public QStackedWidget
{
    Q_OBJECT

public:

    MainWindow();
    ~MainWindow();

    QSize sizeHint () const;
    void keyPressEvent ( QKeyEvent *e);

    void timerEvent(QTimerEvent *);
};

#endif // MAINWINDOW_H

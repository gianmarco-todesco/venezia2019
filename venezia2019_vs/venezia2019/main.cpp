#include <QtGui/QApplication>
#include <QGLFormat>
#include "MainWindow.h"
#include "Viewer.h"
#include "OpenGLPage.h"
#include "PDiskPage.h"
#include <QDebug>


int main(int argc, char *argv[])
{
    qDebug() << "Started!!-----------------------------" << endl;
    QGLFormat fmt;
    fmt.setSampleBuffers(true);
    QGLFormat::setDefaultFormat(fmt);

    QApplication a(argc, argv);
    // PDiskPage w;

    qDebug() << "----------------------------- 1" << endl;
    Viewer w;
    w.show();
    qDebug() << "----------------------------- 2" << endl;
    // w.showFullScreen();
    return a.exec();
}

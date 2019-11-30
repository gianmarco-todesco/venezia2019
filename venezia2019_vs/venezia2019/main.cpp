#include <QtGui/QApplication>
#include <QGLFormat>
#include "Viewer.h"
#include "PDiskPage.h"
#include <QDebug>


int main(int argc, char *argv[])
{
    QGLFormat fmt;
    fmt.setSampleBuffers(true);
    QGLFormat::setDefaultFormat(fmt);

    QApplication a(argc, argv);
    Viewer w;
    w.show();
    // w.showFullScreen();
    return a.exec();
}

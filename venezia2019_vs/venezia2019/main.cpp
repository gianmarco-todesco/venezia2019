#include <QtGui/QApplication>
#include <QGLFormat>
#include "MainWindow.h"
#include "Viewer.h"
#include "OpenGLPage.h"
#include "PDiskPage.h"


int main(int argc, char *argv[])
{
    QGLFormat fmt;
    fmt.setSampleBuffers(true);
    QGLFormat::setDefaultFormat(fmt);

    QApplication a(argc, argv);
    PDiskPage w;
    w.show();
    // w.showFullScreen();
    return a.exec();
}

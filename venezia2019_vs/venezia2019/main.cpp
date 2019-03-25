#include <QtGui/QApplication>
#include "MainWindow.h"
#include "OpenGLPage.h"


int main(int argc, char *argv[])
{

    QApplication a(argc, argv);
    MainWindow w;
    w.show();
    // w.showFullScreen();
    return a.exec();
}

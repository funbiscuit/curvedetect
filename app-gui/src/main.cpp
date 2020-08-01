#include <QApplication>

#include "mainwindow.h"

int main(int argc, char *argv[])
{
    QApplication app(argc, argv);
    QCoreApplication::setOrganizationName("com.github.funbiscuit");
    QCoreApplication::setApplicationName("CurveDetect");
    QCoreApplication::setApplicationVersion("0.1.0");

    MainWindow mainWin;
    mainWin.show();
    return QApplication::exec();
}


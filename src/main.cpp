#include "mainwindow.h"
#include <QApplication>
//#include <stdafx.h>

int main(int argc, char *argv[])
{
    QApplication a(argc, argv);
    MainWindow w;
    w.resize(640, 680);
    w.show();

    return a.exec();
}

#define OPENGL_LEGACY
#include "mainwindow.h"
#include <QApplication>
#include <QStyleFactory>
//#include <stdafx.h>

int main(int argc, char *argv[])
{
	QApplication a(argc, argv);
	MainWindow w;

	w.show();
	a.setStyle(QStyleFactory::create("Fusion"));
	return a.exec();
}

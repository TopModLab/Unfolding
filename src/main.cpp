#include "mainwindow.h"
#include <QApplication>
#include <QStyleFactory>
//#include <stdafx.h>

int main(int argc, char *argv[])
{
	// Enable High DPI Support
	QApplication::setAttribute(Qt::AA_EnableHighDpiScaling);
	QApplication a(argc, argv);
	MainWindow w;
	w.setWindowIcon(QIcon(":/icons/unfolding.png"));
	w.show();
	a.setStyle(QStyleFactory::create("Fusion"));
	return a.exec();
}

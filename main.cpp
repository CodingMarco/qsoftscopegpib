#include "mainwindow.h"
#include "scopeNamespace.h"
#include <QApplication>

int main(int argc, char *argv[])
{
	QApplication a(argc, argv);
	qRegisterMetaType<WaveformPointsVector>("WaveformPointsVector");
	qRegisterMetaType<TIMEBASE_REFERENCE>("TIMEBASE_REFERENCE");
	MainWindow w;
	w.show();
	return a.exec();
}

#include "mainwindow.h"
#include "scopeNamespace.h"
#include <QApplication>

int main(int argc, char *argv[])
{
	QApplication a(argc, argv);
	qRegisterMetaType<WaveformPointsVector>("WaveformPointsVector");
	qRegisterMetaType<MultiChannelWaveformData>("MultiChannelWaveformData");
	qRegisterMetaType<TIMEBASE_REFERENCE>("TIMEBASE_REFERENCE");
	qRegisterMetaType<XYSettings>("XYSettings");
	MainWindow w;
	w.show();
	return a.exec();
}

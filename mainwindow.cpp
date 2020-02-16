#include "mainwindow.h"
#include "ui_mainwindow.h"
#include "scope.h"
#include "scopeNamespace.h"
#include <QDebug>
#include <unistd.h>

MainWindow::MainWindow(QWidget *parent)
	: QMainWindow(parent)
	, ui(new Ui::MainWindow)
{
	ui->setupUi(this);
	scope = new Scope(7);
//	scope->setPoints(POINTS_512);
//	scope->setFormat(WAVEFORM_FORMAT_WORD);
//	scope->setAcquireType(ACQUIRE_TYPE_NORMAL);

	scope->writeCmd(":autoscale");
	QVector<short> data = scope->getWaveformData();
	qDebug() << "First data: " << data[0];

	//	for(int i = 0; i < 200; i++)
	//	{
	//		scope->writeCmd(":RUN");
	//		usleep(10e3);
	//		scope->writeCmd(":STOP");
	//		usleep(10e3);
	//	}
}

MainWindow::~MainWindow()
{
	delete ui;
	delete scope;
}


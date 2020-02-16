#include "mainwindow.h"
#include "ui_mainwindow.h"
#include "scope.h"
#include "scopeNamespace.h"
#include <QDebug>

MainWindow::MainWindow(QWidget *parent)
	: QMainWindow(parent)
	, ui(new Ui::MainWindow)
{
	ui->setupUi(this);
	scope = new Scope(7);
	scope->setPoints(POINTS_512);
	scope->setFormat(WAVEFORM_FORMAT_WORD);
	scope->setAcquireType(ACQUIRE_TYPE_NORMAL);

	scope->writeCmd(":autoscale");
	QVector<short> data = scope->getWaveformData();
	qDebug() << "Hello";
	qDebug() << "Hello2 " << data[0] << " fin";
}

MainWindow::~MainWindow()
{
	delete ui;
	delete scope;
}


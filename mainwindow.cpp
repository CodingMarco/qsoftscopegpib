#include "mainwindow.h"
#include "ui_mainwindow.h"
#include "scope.h"
#include "scopeNamespace.h"
#include <QDebug>
#include <QVector>
#include <QTimer>
#include <unistd.h>
#include <qwt/qwt_plot.h>
#include <qwt/qwt_plot_curve.h>
#include <qwt/qwt_scale_engine.h>

MainWindow::MainWindow(QWidget *parent)
	: QMainWindow(parent),
	  ui(new Ui::MainWindow)
{
	ui->setupUi(this);
	wftimer = new QTimer(this);

	connect(wftimer, SIGNAL(timeout()), this, SLOT(plotWaveform()));

	waveformCurve = new QwtPlotCurve("Waveform");
	waveformCurve->attach(ui->qwtPlot);
	ui->qwtPlot->axisScaleEngine(QwtPlot::xBottom)->setAttribute(QwtScaleEngine::Floating,true);
	ui->qwtPlot->setAxisScale(QwtPlot::yLeft, 0, 32640);
}

MainWindow::~MainWindow()
{
	delete ui;
}

void MainWindow::plotWaveform()
{
}

void MainWindow::on_cmdStart_clicked()
{
	wftimer->start(1);
}

void MainWindow::on_cmdConnect_clicked()
{
}

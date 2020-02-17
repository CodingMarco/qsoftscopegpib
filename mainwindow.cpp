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
	: QMainWindow(parent)
	, ui(new Ui::MainWindow)
{
	ui->setupUi(this);
	scope = new Scope(7);
	wftimer = new QTimer(this);

	connect(wftimer, SIGNAL(timeout()), this, SLOT(plotWaveform()));

	waveformCurve = new QwtPlotCurve("Waveform");
	waveformCurve->attach(ui->qwtPlot);
	ui->qwtPlot->axisScaleEngine(QwtPlot::xBottom)->setAttribute(QwtScaleEngine::Floating,true);
	ui->qwtPlot->setAxisScale(QwtPlot::yLeft, 0, 32000);

	scope->setPoints(POINTS_32768);
	scope->writeCmd(":TIMEBASE:SAMPLE:CLOCK 5E6");
}

MainWindow::~MainWindow()
{
	delete ui;
	delete scope;
}

void MainWindow::plotWaveform()
{
	QVector<ushort> yshort = scope->getWaveformData();
	QVector<double> x;
	QVector<double> y;
	x.reserve(scope->getPoints());
	y.reserve(scope->getPoints());
	for(int i = 0; i < scope->getPoints(); i++)
	{
		x.append(i);
		y.append(yshort[i]);
	}
	waveformCurve->setSamples(x, y);

	ui->qwtPlot->replot();
}


void MainWindow::on_cmdRefresh_clicked()
{
	wftimer->start(1);
}

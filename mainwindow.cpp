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
	wftimer = new QTimer(this);

	connect(wftimer, SIGNAL(timeout()), this, SLOT(plotWaveform()));

	waveformCurve = new QwtPlotCurve("Waveform");
	waveformCurve->attach(ui->qwtPlot);
	ui->qwtPlot->axisScaleEngine(QwtPlot::xBottom)->setAttribute(QwtScaleEngine::Floating,true);
	ui->qwtPlot->setAxisScale(QwtPlot::yLeft, 0, 32640);

	//scope->setPoints(POINTS_512);
	//scope->autoscale();
	//scope->writeCmd(":TIMEBASE:SAMPLE:CLOCK 5E6");
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
	x.reserve(scope->points());
	y.reserve(scope->points());
	double secondsPerSample = scope->timebaseRange()/scope->points();
	double voltsPerY = scope->query("CHANNEL1:RANGE?").toDouble()/32640;
	for(int i = 0; i < scope->points(); i++)
	{
		x.append(double(i)*secondsPerSample);
		y.append(double(yshort[i])*voltsPerY);
	}
	waveformCurve->setSamples(x, y);

	ui->qwtPlot->replot();
}

void MainWindow::on_cmdStart_clicked()
{
	wftimer->start(1);
}

void MainWindow::on_cmdConnect_clicked()
{
	scope = new Scope(7);
	ui->cmdSend->setEnabled(true);
	ui->cmdStart->setEnabled(true);
}

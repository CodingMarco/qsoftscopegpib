#include "mainwindow.h"
#include "ui_mainwindow.h"
#include "scopeNamespace.h"
#include "connectdialog.h"
#include "customAxisScaleDraw.h"

#include <unistd.h>
#include <qwt/qwt_plot.h>
#include <qwt/qwt_plot_curve.h>
#include <qwt/qwt_scale_engine.h>
#include <qwt/qwt_plot_grid.h>

#include <QDebug>
#include <QVector>
#include <QTimer>
#include <QMessageBox>
#include <QtConcurrent/QtConcurrentRun>
#include <QtMath>

MainWindow::MainWindow(QWidget *parent)
	: QMainWindow(parent),
	  ui(new Ui::MainWindow)
{
	ui->setupUi(this);
	scope.moveToThread(&scopeThread);

	//connect(&scope, SIGNAL(timebaseRangeUpdated(double)), this, SLOT(updateTimebaseRange(double)));

	// Buttons and other controls
	connect(ui->cmdStart, &QPushButton::clicked, &scope, &Scope::startWaveformUpdate);
	connect(ui->cmdStop, &QPushButton::clicked, &scope, &Scope::stopWaveformUpdate);
	connect(ui->cmdZoomIn, &QPushButton::clicked, &scope, &Scope::zoomIn);
	connect(ui->cmdZoomOut, &QPushButton::clicked, &scope, &Scope::zoomOut);
	connect(ui->cmdAutoscale, &QPushButton::clicked, &scope, &Scope::autoscale);
	connect(ui->comboBoxPoints, SIGNAL(currentIndexChanged(QString)), &scope, SLOT(setPoints(QString)));

	// Scope thread stuff
	connect(&scopeThread, SIGNAL(started()), &scope, SLOT(initializeThreadRelatedStuff()));

	// Waveform plot
	connect(&scope, SIGNAL(waveformUpdated(WaveformPointsVector)), this, SLOT(plotWaveform(WaveformPointsVector)));
	connect(ui->qwtPlot, &WaveformPlot::mouseScrolled, this, &MainWindow::zoom);

	scopeThread.start();

	waveformCurve = new QwtPlotCurve("Waveform");
	waveformCurve->setStyle(QwtPlotCurve::Lines);
	waveformCurve->setPen(QColor::fromRgb(255,100,0), 1);
	waveformCurve->attach(ui->qwtPlot);
	ui->qwtPlot->axisScaleEngine(QwtPlot::xBottom)->setAttribute(QwtScaleEngine::Floating, false);
	ui->qwtPlot->setAxisScaleDraw(QwtPlot::xBottom, new TimebaseScaleDraw);
	ui->qwtPlot->setAxisScaleDraw(QwtPlot::yLeft, new VoltageScaleDraw);
	ui->qwtPlot->setAxisScale(QwtPlot::yLeft, -0.02, 0.02);

	// Grid
	QwtPlotGrid *grid = new QwtPlotGrid;
	grid->enableXMin(true);
	grid->setMajorPen(QPen(QColor(255, 255, 255, 100), 0, Qt::DotLine));
	grid->setMinorPen(QPen(QColor(100, 100, 100, 100), 0, Qt::DotLine));
	grid->attach(ui->qwtPlot);

	autoconnect();
}

MainWindow::~MainWindow()
{
	delete ui;
}

void MainWindow::closeEvent(QCloseEvent *event)
{
	QMetaObject::invokeMethod(&scope, "stopWaveformUpdate", Qt::BlockingQueuedConnection);
	scope.closeInstrument();
	scopeThread.quit();
	event->accept();
}

bool MainWindow::autoconnect()
{
	scope.openInstrument(7);
	scope.setFormat(WAVEFORM_FORMAT_WORD);
	scope.setPoints(POINTS_512);
	scope.setAcquireType(ACQUIRE_TYPE_NORMAL);
	scope.setTimebaseReference(CENTER);
	ui->qwtPlot->axisScaleEngine(QwtPlot::xBottom)->setAttribute(QwtScaleEngine::Symmetric, true);
	timebaseRange = scope.maximumTimebaseRange();
	return true;
}

void MainWindow::plotWaveform(WaveformPointsVector waveformData)
{
	waveformCurve->setSamples(waveformData);
	ui->qwtPlot->replot();
}

void MainWindow::on_actionConnect_triggered()
{
	ConnectDialog connectDialog(scope, this);
	connectDialog.exec();
}

void MainWindow::on_actionInfo_triggered()
{
	if(scope.isOpen())
		QMessageBox::information(this, "IDN", scope.idn());
	else
		QMessageBox::information(this, "Not connected", "Error: Scope not connected!");
}

void MainWindow::on_cmdQuery_clicked()
{
	if(scope.isOpen())
	{
		QString response;
		QMetaObject::invokeMethod(&scope, "query", Qt::BlockingQueuedConnection, Q_RETURN_ARG(QString, response),
								  Q_ARG(QString, ui->lineEditCommand->text()));
		QMessageBox::information(this, "Response", response);
	}
	else
		QMessageBox::information(this, "Not connected", "Error: Scope not connected!");
}

void MainWindow::on_cmdSend_clicked()
{
	if(scope.isOpen())
		QMetaObject::invokeMethod(&scope, "writeCmd", Q_ARG(QString, ui->lineEditCommand->text()));
	else
		QMessageBox::information(this, "Not connected", "Error: Scope not connected!");
}

void MainWindow::setTimebaseRange(double range)
{
	ui->lblTimebaseRangeNumber->setText(QString::number(range));
	switch(scope.timebaseReference()) {
		case LEFT:
			ui->qwtPlot->setAxisScale(QwtPlot::xBottom, 0, range);
			break;
		case CENTER:
			ui->qwtPlot->setAxisScale(QwtPlot::xBottom, -(range/2), range/2);
			break;
		case RIGHT:
			ui->qwtPlot->setAxisScale(QwtPlot::xBottom, -range, 0);
			break;
	}
	ui->qwtPlot->replot();
}

void MainWindow::on_comboBoxReference_currentIndexChanged(int reference_mode)
{
	QMetaObject::invokeMethod(&scope, "setTimebaseReference",
							  Q_ARG(TIMEBASE_REFERENCE, TIMEBASE_REFERENCE(reference_mode)));
	if(reference_mode == CENTER)
		ui->qwtPlot->axisScaleEngine(QwtPlot::xBottom)->setAttribute(QwtScaleEngine::Symmetric, true);
	else
		ui->qwtPlot->axisScaleEngine(QwtPlot::xBottom)->setAttribute(QwtScaleEngine::Symmetric, false);
	//updateTimebaseRange(scope.timebaseRange());
}

void MainWindow::on_cmdStop_clicked()
{
	if(!scope.waveformUpdateActive())
		QMetaObject::invokeMethod(&scope, "singleWaveformUpdate");
}

void MainWindow::zoom(int amount)
{
	double absAmount = abs(amount);
	if(amount > 0)
		timebaseRange *= (absAmount * 1.2);
	else
		timebaseRange *= (absAmount * 0.8);
	setTimebaseRange(timebaseRange);
}

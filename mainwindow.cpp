#include "mainwindow.h"
#include "ui_mainwindow.h"
#include "scopeNamespace.h"
#include "connectdialog.h"
#include "customAxisScaleDraw.h"
#include "commonFunctions.h"

#include <qwt/qwt_plot.h>
#include <qwt/qwt_plot_curve.h>
#include <qwt/qwt_scale_engine.h>
#include <qwt/qwt_plot_grid.h>
#include <qwt/qwt_curve_fitter.h>
#include <qwt/qwt_spline.h>

#include <QDebug>
#include <QVector>
#include <QTimer>
#include <QMessageBox>
#include <QtMath>

MainWindow::MainWindow(QWidget *parent)
	: QMainWindow(parent),
	  ui(new Ui::MainWindow)
{
	ui->setupUi(this);
	scope.moveToThread(&scopeThread);

	// Buttons and other controls
	connect(ui->cmdStart, &QPushButton::clicked, &scope, &Scope::startWaveformUpdate);
	connect(ui->cmdStop, &QPushButton::clicked, &scope, &Scope::stopWaveformUpdate);
	connect(ui->cmdAutoscale, &QPushButton::clicked, &scope, &Scope::autoscale);
	connect(ui->cmdZoomIn, SIGNAL(clicked()), &scope, SLOT(zoomIn()));
	connect(ui->cmdZoomOut, SIGNAL(clicked()), &scope, SLOT(zoomOut()));
	connect(&scope, &Scope::zoomed, this, &MainWindow::setTimebaseRange);
	connect(&scope, &Scope::autoscaleComplete, this, &MainWindow::adjustXYAfterAutoscale);
	connect(ui->comboBoxPoints, SIGNAL(currentIndexChanged(QString)), &scope, SLOT(setPoints(QString)));

	// Scope thread stuff
	connect(&scopeThread, SIGNAL(started()), &scope, SLOT(initializeThreadRelatedStuff()));

	// Waveform plot
	connect(&scope, SIGNAL(waveformUpdated(MultiChannelWaveformData)), this, SLOT(plotWaveforms(MultiChannelWaveformData)));
	connect(ui->qwtPlot, &WaveformPlot::mouseScrolled, this, &MainWindow::zoomTimebase);
	connect(ui->qwtPlot, &WaveformPlot::mouseWithShiftScrolled, this, &MainWindow::zoomVertical);

	scopeThread.start();

	waveformCurve = new QwtPlotCurve("Waveform");
	waveformCurve->setStyle(QwtPlotCurve::Lines);
	waveformCurve->setPen(QColor::fromRgb(255,100,255), 1);
	waveformCurve->attach(ui->qwtPlot);
	ui->qwtPlot->setAxisScaleDraw(QwtPlot::xBottom, new TimebaseScaleDraw);
	ui->qwtPlot->setAxisScaleDraw(QwtPlot::yLeft, new VoltageScaleDraw);

	// Grid
	QwtPlotGrid *grid = new QwtPlotGrid;
	grid->enableXMin(true);
	grid->setMajorPen(QPen(QColor(255, 255, 255, 100), 0, Qt::DotLine));
	grid->setMinorPen(QPen(QColor(100, 100, 100, 100), 0, Qt::DotLine));
	grid->attach(ui->qwtPlot);

	// Panner (move curve with mouse)
	panner = new QwtPlotPanner(ui->qwtPlot->canvas());
	panner->setMouseButton(Qt::LeftButton);

	// Marker
//	marker = new QwtPlotMarker("X1");
//	marker->setLineStyle(QwtPlotMarker::VLine);
//	marker->setLinePen(QColor::fromRgb(100,100,255), 2);
//	marker->attach(ui->qwtPlot);

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
	scope.initializeScope();
	ui->qwtPlot->axisScaleEngine(QwtPlot::xBottom)->setAttribute(QwtScaleEngine::Symmetric, true);
	timebaseRange = scope.nextLowerTimebaseRange();
	QMetaObject::invokeMethod(&scope, "getChannelRange", Qt::BlockingQueuedConnection, Q_RETURN_ARG(double, channelRange));
	QMetaObject::invokeMethod(&scope, "getChannelOffset", Qt::BlockingQueuedConnection, Q_RETURN_ARG(double, channelOffset));
	updateChannelRange();
	return true;
}

void MainWindow::plotWaveforms(MultiChannelWaveformData waveformData)
{
	waveformCurve->setSamples(waveformData[0]);
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
	timebaseRange = range;
	ui->lblTimebaseRangeNumber->setText(CommonFunctions::toSiValue(range, "s"));

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
}

void MainWindow::on_cmdStop_clicked()
{
	if(!scope.waveformUpdateActive())
		QMetaObject::invokeMethod(&scope, "singleWaveformUpdate");
}

void MainWindow::zoomTimebase(int amount)
{
	double absAmount = abs(amount);
	double newTimebaseRange = 0;
	if(amount < 0)
		newTimebaseRange = timebaseRange * absAmount * 1.05;
	else if(amount > 0)
		newTimebaseRange = timebaseRange * absAmount * 0.9;

	setTimebaseRange(newTimebaseRange);

	QMetaObject::invokeMethod(&scope, "autoAdjustSampleRate", Q_ARG(double, newTimebaseRange));
}

void MainWindow::zoomVertical(int amount)
{
	double oldChannelRange = channelRange;
	double absAmount = abs(amount);
	if(amount < 0)
		channelRange *= absAmount * 1.1;
	else
		channelRange *= absAmount * 0.9;
	QMetaObject::invokeMethod(&scope, "autoAdjustChannelRange",
							  Q_ARG(double, oldChannelRange), Q_ARG(double, channelRange));
	updateChannelRange();
}

void MainWindow::updateChannelRange()
{
	ui->qwtPlot->setAxisScale(QwtPlot::yLeft, -channelRange/2+channelOffset, channelRange/2+channelOffset);
	ui->qwtPlot->replot();
}

void MainWindow::on_checkBoxACLF_stateChanged()
{
	if(ui->checkBoxACLF->isChecked())
	{
		ui->qwtPlot->setAxisScale(QwtPlot::yLeft, -0.03, 0.03);
		QMetaObject::invokeMethod(&scope, "toggleAcCouplingAndLfReject", Q_ARG(bool, true));
	}
	else
	{
		ui->qwtPlot->setAxisScale(QwtPlot::yLeft, -1.5, 0.5);
		QMetaObject::invokeMethod(&scope, "toggleAcCouplingAndLfReject", Q_ARG(bool, false));
	}
}

void MainWindow::adjustXYAfterAutoscale(XYSettings autoscaleResult)
{
	setTimebaseRange(autoscaleResult.timebaseRange);
	channelRange = autoscaleResult.channelRange;
	channelOffset = autoscaleResult.channelOffset;
	updateChannelRange();
}

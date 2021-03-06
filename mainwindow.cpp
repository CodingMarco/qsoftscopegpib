#include "mainwindow.h"
#include "ui_mainwindow.h"
#include "scopeNamespace.h"
#include "connectdialog.h"
#include "customAxisScaleDraw.h"
#include "commonFunctions.h"
#include "customQwtPlotCurve.h"

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
	connect(&scope, SIGNAL(waveformUpdated(MultiChannelWaveformData, ScopeSettings)), this, SLOT(plotWaveforms(MultiChannelWaveformData, ScopeSettings)));
	connect(ui->qwtPlot, &WaveformPlot::mouseScrolled, this, &MainWindow::zoomTimebase);
	connect(ui->qwtPlot, &WaveformPlot::mouseWithShiftScrolled, this, &MainWindow::zoomVertical);

	scopeThread.start();

	// Initialize waveform curves
	functionCurves.append(new CustomQwtPlotCurve);
	functionCurves.last()->setStyle(QwtPlotCurve::Lines);
	functionCurves.last()->setPen(QColor::fromRgb(255,20,147), 1);
	functionCurves.last()->attach(ui->qwtPlot);

	for(int i = 0; i < 4; i++)
	{
		waveformCurves.append(new CustomQwtPlotCurve);
		waveformCurves.last()->setStyle(QwtPlotCurve::Lines);

		QwtSplineCurveFitter* fitter = new QwtSplineCurveFitter();
		fitter->setFitMode(QwtSplineCurveFitter::Spline);
		fitter->setSplineSize(3000);

		waveformCurves.last()->setCurveFitter(fitter);
		waveformCurves.last()->setCurveAttribute(QwtPlotCurve::Fitted);


		waveformCurves.last()->attach(ui->qwtPlot);
	}

	waveformCurves[0]->setPen(QColor::fromRgb(255,255,000), 1);
	waveformCurves[1]->setPen(QColor::fromRgb(000,255,000), 1);
	waveformCurves[2]->setPen(QColor::fromRgb(100,100,255), 1);
	waveformCurves[3]->setPen(QColor::fromRgb(255,100,100), 1);

	ui->groupBoxCh1->setStyleSheet("QGroupBox::title { color: rgb(255,255,000); }");
	ui->groupBoxCh2->setStyleSheet("QGroupBox::title { color: rgb(000,255,000); }");
	ui->groupBoxCh3->setStyleSheet("QGroupBox::title { color: rgb(100,100,255); }");
	ui->groupBoxCh4->setStyleSheet("QGroupBox::title { color: rgb(255,100,100); }");

	connect(ui->chkShowCh1, SIGNAL(clicked(bool)), waveformCurves[1-1], SLOT(setVisible(bool)));
	connect(ui->chkShowCh2, SIGNAL(clicked(bool)), waveformCurves[2-1], SLOT(setVisible(bool)));
	connect(ui->chkShowCh3, SIGNAL(clicked(bool)), waveformCurves[3-1], SLOT(setVisible(bool)));
	connect(ui->chkShowCh4, SIGNAL(clicked(bool)), waveformCurves[4-1], SLOT(setVisible(bool)));

	// Initialize custom axis labels
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

	autoconnect();
}

MainWindow::~MainWindow()
{
	delete ui;
}

void MainWindow::closeEvent(QCloseEvent *event)
{
	QMetaObject::invokeMethod(&scope, "stopWaveformUpdate", Qt::BlockingQueuedConnection);
	scope.writeCmd(":DISP:SCREEN ON");
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

void MainWindow::plotWaveforms(MultiChannelWaveformData waveformData, ScopeSettings scopeSettings)
{
	//double frequency = measurements.getFrequency(waveformData[0], scopeSettings.sampleRate);
	//double period = double(1)/frequency;

	//ui->lblFreqNumber->setText(CommonFunctions::toSiValue(frequency, "Hz"));
	//ui->lblPeriodNumber->setText(CommonFunctions::toSiValue(period, "s"));

	ui->lblSampleRateNumber->setText(CommonFunctions::toSiValue(scopeSettings.sampleRate, "Sa/s"));

	static int fpsCounter = 0;
	fpsCounter++;
	if(fpsTimer.elapsed() > 1000)
	{
		ui->lblFpsNumber->setNum(fpsCounter);
		fpsCounter = 0;
		fpsTimer.restart();
	}

	for(int i = 0; i < waveformCurves.size(); i++)
	{
		waveformCurves[i]->setSamples(waveformData[i]);
	}

	// Function: 1-2
//	if(ui->chkEnabledCh1->isChecked() && ui->chkEnabledCh2->isChecked())
//	{
//		functionCurves[0]->setSamples(CommonFunctions::getFunctionWaveform(waveformData[1-1], waveformData[2-1], MINUS));
//	}

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
	double absAmount = abs(amount);
	if(amount < 0)
		channelRange *= absAmount * 1.1;
	else
		channelRange *= absAmount * 0.9;
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

void MainWindow::on_cmdStart_clicked()
{
	fpsTimer.restart();
}

void MainWindow::on_chkEnabledCh1_stateChanged(int checkState)
{
	QMetaObject::invokeMethod(&scope, "setChannelEnabled", Q_ARG(int, 1), Q_ARG(bool, checkState == Qt::Checked));
}

void MainWindow::on_chkEnabledCh2_stateChanged(int checkState)
{
	QMetaObject::invokeMethod(&scope, "setChannelEnabled", Q_ARG(int, 2), Q_ARG(bool, checkState == Qt::Checked));
}

void MainWindow::on_chkEnabledCh3_stateChanged(int checkState)
{
	QMetaObject::invokeMethod(&scope, "setChannelEnabled", Q_ARG(int, 3), Q_ARG(bool, checkState == Qt::Checked));
}

void MainWindow::on_chkEnabledCh4_stateChanged(int checkState)
{
	QMetaObject::invokeMethod(&scope, "setChannelEnabled", Q_ARG(int, 4), Q_ARG(bool, checkState == Qt::Checked));
}

void MainWindow::on_cmdTriggerLevelSet_clicked()
{
	QMetaObject::invokeMethod(&scope, "setTriggerLevel",
		  Q_ARG(double, CommonFunctions::spinBoxAndComboBoxToVoltage(ui->spinBoxTriggerLevel->value(),
																	 ui->comboBoxTriggerLevel->currentIndex())));
}

void MainWindow::on_cmdTriggerHoldoffSet_clicked()
{
	QMetaObject::invokeMethod(&scope, "setTriggerHoldoff",
		  Q_ARG(double, CommonFunctions::spinBoxAndComboBoxToVoltage(ui->spinBoxTriggerHoldoff->value(),
																	 ui->comboBoxTriggerHoldoff->currentIndex())));
}

void MainWindow::on_comboBox_currentIndexChanged(const QString &channel)
{
	QMetaObject::invokeMethod(&scope, "setTriggerSourceChannel", Q_ARG(int, channel.toInt()));
}

void MainWindow::on_checkBoxNoiseReject_stateChanged(int checkState)
{
	QMetaObject::invokeMethod(&scope, "enableTriggerNoiseReject", Q_ARG(bool, checkState == Qt::Checked));
}

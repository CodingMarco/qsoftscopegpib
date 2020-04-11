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

MainWindow::MainWindow(QWidget *parent)
	: QMainWindow(parent),
	  ui(new Ui::MainWindow)
{
	ui->setupUi(this);
	wftimer = new QTimer(this);

	connect(wftimer, SIGNAL(timeout()), this, SLOT(plotWaveform()));
	connect(&scope, SIGNAL(timebaseRangeUpdated(double)), this, SLOT(updateTimebaseRange(double)));

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
	on_cmdStop_clicked();
	scope.closeInstrument();
	event->accept();
}

bool MainWindow::autoconnect()
{
	scope.openInstrument(7);
	scope.setFormat(WAVEFORM_FORMAT_WORD);
	scope.setPoints(POINTS_512);
	scope.setAcquireType(ACQUIRE_TYPE_NORMAL);
	scope.setTimebaseReference(CENTER);
	scope.writeCmd(":TIMEBASE:SAMPlE:CLOCK AUTO");
	scope.initializeParameters();
	updateTimebaseRange(scope.timebaseRange());
	ui->qwtPlot->axisScaleEngine(QwtPlot::xBottom)->setAttribute(QwtScaleEngine::Symmetric, true);
	return true;
}

void MainWindow::plotWaveform()
{
	auto waveformData = scope.digitizeAndGetPoints();
	waveformCurve->setSamples(waveformData);
	ui->qwtPlot->replot();
}

void MainWindow::on_cmdStart_clicked()
{
	wftimer->start();
}

void MainWindow::on_cmdStop_clicked()
{
	if(wftimer->isActive())
		wftimer->stop();
	else
		plotWaveform();
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
		QMessageBox::information(this, "Response", scope.query(ui->lineEditCommand->text()));
	else
		QMessageBox::information(this, "Not connected", "Error: Scope not connected!");
}

void MainWindow::on_cmdSend_clicked()
{
	scope.writeCmd(ui->lineEditCommand->text());
}

void MainWindow::on_cmdAutoscale_clicked()
{
	scope.autoscale();
}

void MainWindow::on_comboBoxPoints_currentIndexChanged(const QString &points)
{
	scope.setPoints(POINTS(points.toInt()));
}

void MainWindow::on_cmdZoomIn_clicked()
{
	scope.zoomIn();
}

void MainWindow::on_cmdZoomOut_clicked()
{
	scope.zoomOut();
}

void MainWindow::updateTimebaseRange(double range)
{
	ui->lblTimebaseRangeNumber->setText(QString::number(range, 'e', 0));
	// compensate number of points. On scope screen, always 512 points are displayed. We want to display them all.
	range *= (scope.points() / 512);
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
}

void MainWindow::on_comboBoxReference_currentIndexChanged(int reference_mode)
{
	scope.setTimebaseReference(TIMEBASE_REFERENCE(reference_mode));
	if(reference_mode == CENTER)
		ui->qwtPlot->axisScaleEngine(QwtPlot::xBottom)->setAttribute(QwtScaleEngine::Symmetric, true);
	else
		ui->qwtPlot->axisScaleEngine(QwtPlot::xBottom)->setAttribute(QwtScaleEngine::Symmetric, false);
	updateTimebaseRange(scope.timebaseRange());
}

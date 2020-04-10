#include "mainwindow.h"
#include "ui_mainwindow.h"
#include "scope.h"
#include "scopeNamespace.h"
#include "connectdialog.h"

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

	waveformCurve = new QwtPlotCurve("Waveform");
	waveformCurve->setPen(QColor::fromRgb(255,100,0), 1);
	waveformCurve->attach(ui->qwtPlot);
	ui->qwtPlot->axisScaleEngine(QwtPlot::xBottom)->setAttribute(QwtScaleEngine::Floating, true);
	ui->qwtPlot->axisScaleEngine(QwtPlot::xBottom)->setAttribute(QwtScaleEngine::Symmetric, true);
	ui->qwtPlot->setAxisScale(QwtPlot::yLeft, -3, 3, 0.5);

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
	scope.closeInstrument();
	event->accept();
}

bool MainWindow::autoconnect()
{
	scope.openInstrument(7);
	scope.setFormat(WAVEFORM_FORMAT_WORD);
	scope.setPoints(POINTS_512);
	scope.setAcquireType(ACQUIRE_TYPE_NORMAL);
	scope.writeCmd(":TIMEBASE:SAMPlE:CLOCK AUTO");
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
	wftimer->stop();
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

void MainWindow::on_comboBox_currentIndexChanged(const QString &points)
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

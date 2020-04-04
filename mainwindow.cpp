#include "mainwindow.h"
#include "ui_mainwindow.h"
#include "scope.h"
#include "scopeNamespace.h"
#include "connectdialog.h"
#include "fraDialog.h"

#include <unistd.h>
#include <qwt/qwt_plot.h>
#include <qwt/qwt_plot_curve.h>
#include <qwt/qwt_scale_engine.h>

#include <QDebug>
#include <QVector>
#include <QTimer>
#include <QMessageBox>

MainWindow::MainWindow(QWidget *parent)
	: QMainWindow(parent),
	  ui(new Ui::MainWindow),
	  instrumentConnection(new InstrumentConnection)
{
	ui->setupUi(this);
	wftimer = new QTimer(this);

	connect(wftimer, SIGNAL(timeout()), this, SLOT(plotWaveform()));

	waveformCurve = new QwtPlotCurve("Waveform");
	waveformCurve->setPen(QColor::fromRgb(255,100,0), 2);
	waveformCurve->attach(ui->qwtPlot);
	ui->qwtPlot->axisScaleEngine(QwtPlot::xBottom)->setAttribute(QwtScaleEngine::Floating,true);
	autoconnect();
}

MainWindow::~MainWindow()
{
	delete ui;
}

void MainWindow::closeEvent(QCloseEvent *event)
{
	instrumentConnection->closeInstrument();
	event->accept();
}

bool MainWindow::autoconnect()
{
	QString visaAddress = instrumentConnection->getAvailableVisaInstruments().first();
	bool connectStatus = instrumentConnection->connectToInstrument(visaAddress);
	if(connectStatus)
	{
		scope = new Scope(instrumentConnection);
	}
	return connectStatus;
}

void MainWindow::plotWaveform()
{
	scope->setPoints(POINTS_32768);
	QVector<ushort> yshort = scope->getWaveformData();
	QVector<double> x;
	QVector<double> y;
	x.reserve(scope->points());
	y.reserve(scope->points());

	//double secondsPerSample = scope->timebaseRange()/scope->points();
	//double voltsPerY = scope->channelRange()/32768;
	//ui->qwtPlot->setAxisScale(QwtPlot::yLeft, 0, 0.1);

	auto preamble = scope->waveformPreamble();

	for(int i = 0; i < scope->points(); i++)
	{
		x.append((double(i)-preamble["xreference"]) * preamble["xincrement"] + preamble["xorigin"]);
		y.append(((double(yshort[i])-preamble["yreference"]) * preamble["yincrement"]) + preamble["yorigin"]);
	}
	waveformCurve->setSamples(x, y);

	ui->qwtPlot->replot();
}

void MainWindow::on_cmdStart_clicked()
{
	wftimer->start(1);
}

void MainWindow::on_cmdStop_clicked()
{
	wftimer->stop();
}

void MainWindow::on_actionConnect_triggered()
{
	ConnectDialog connectDialog(instrumentConnection, this);
	if(connectDialog.exec() == QDialog::Accepted)
	{
		if(scope != nullptr)
			delete scope;
		scope = new Scope(instrumentConnection);
	}
}

void MainWindow::on_actionInfo_triggered()
{
	if(scope != nullptr)
		QMessageBox::information(this, "IDN", scope->idn());
	else
		QMessageBox::information(this, "Not connected", "Error: Scope not connected!");
}

void MainWindow::on_actionFequencyResponse_Analysis_triggered()
{
	if(scope != nullptr)
	{
		FraDialog fraDialog(scope, this);
		fraDialog.exec();
	}
	else
		QMessageBox::information(this, "Not connected", "Error: Scope not connected!");
}

void MainWindow::on_cmdSend_clicked()
{
	if(scope != nullptr)
		QMessageBox::information(this, "Response", instrumentConnection->query(ui->lineEditCommand->text()));
	else
		QMessageBox::information(this, "Not connected", "Error: Scope not connected!");
}

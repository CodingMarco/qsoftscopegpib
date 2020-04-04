#include "fraDialog.h"
#include "ui_fradialog.h"
#include <QtMath>
#include <QDebug>
#include <qwt_plot_curve.h>
#include <qwt_scale_engine.h>
#include <qwt_curve_fitter.h>
#include <qwt_plot_grid.h>
#include <unistd.h>

FraDialog::FraDialog(Scope *m_scope, QWidget *parent) :
	QDialog(parent),
	ui(new Ui::FraDialog),
	scope(m_scope)
{
	ui->setupUi(this);
	fraCurve = new QwtPlotCurve("Frequency response");
	fraCurve->setPen(QColor::fromRgb(255,100,0), 2);

	//fraCurve->setStyle(QwtPlotCurve::CurveStyle::Dots);

	// Interpolation
	fraCurve->setCurveAttribute(QwtPlotCurve::CurveAttribute::Fitted);
	fraCurve->setCurveFitter(new QwtSplineCurveFitter);

	// Grid
	QwtPlotGrid *grid = new QwtPlotGrid;
	grid->enableXMin( true );
	grid->setMajorPen(QPen(QColor(255, 255, 255, 100), 0, Qt::DotLine));
	grid->setMinorPen(QPen(QColor(100, 100, 100, 100), 0, Qt::DotLine));
	grid->attach(ui->plotFra);

	fraCurve->attach(ui->plotFra);
	ui->plotFra->setAxisScaleEngine(QwtPlot::xBottom, new QwtLogScaleEngine);
	ui->plotFra->axisScaleEngine(QwtPlot::xBottom)->setAttribute(QwtScaleEngine::Floating, true);
	//ui->plotFra->setAxisScale(QwtPlot::yLeft, 0, 6);

	// Axis
	ui->plotFra->setAxisTitle(QwtPlot::xBottom, "Frequency [Hz]");
	ui->plotFra->setAxisTitle(QwtPlot::yLeft, "Amplitude [V]");
}

FraDialog::~FraDialog()
{
	delete ui;
}

void FraDialog::on_cmdStart_clicked()
{
	int start_freq = ui->spinBoxStartFreq->value() * pow(1000, ui->frequencyStartMultiplier->currentIndex());
	int stop_freq = ui->spinBoxStopFreq->value() * pow(1000, ui->frequencyStopMultiplier->currentIndex());
	int step_size = (stop_freq - start_freq) / ui->spinBoxNrOfSteps->value();
	qDebug() << "Starting FRA from" << start_freq << "Hz to" << stop_freq << "Hz with a step size of" << step_size;

	amplitudes.clear();
	amplitudesDb.clear();
	frequencies.clear();
	amplitudes.reserve((stop_freq - start_freq) / step_size);
	amplitudesDb.reserve((stop_freq - start_freq) / step_size);
	frequencies.reserve((stop_freq - start_freq) / step_size);

	// General setup
	scope->writeCmd(":WGEN1:FUNC SIN");
	scope->writeCmd(":WGEN1:VOLTAGE 5");
	scope->writeCmd(QString(":WGEN1:FREQ ") + QString::number(start_freq));
	scope->writeCmd(":WGEN1:OUTPUT ON");
	scope->writeCmd(":MEASURE:SOURCE CHANNEL1");

	scope->writeCmd(":CHANNEL1:RANGE 8");
	scope->writeCmd(":TIMEBASE:REFERENCE LEFT");

	for(int i = 0; i < ui->spinBoxNrOfSteps->value(); i++)
	{
		double scale = (log(stop_freq)-log(start_freq)) / ui->spinBoxNrOfSteps->value();
		double current_frequency = exp(log(start_freq) + scale*i);

		scope->writeCmd(QString(":TIMEBASE:RANGE ") + QString::number(double(2) / current_frequency));
		scope->writeCmd(QString(":WGEN1:FREQ ") + QString::number(current_frequency));

		frequencies.append(current_frequency);
		double amplitude = scope->query(":MEASURE:VAMP?").toDouble();
		amplitudes.append(amplitude);
		double db = 20*log(amplitude/5);
		amplitudesDb.append(db);

		if(amplitude > 0.04)
			scope->writeCmd(QString(":CHANNEL1:RANGE ") + QString::number(1.5 * amplitude));
	}
	// Last point with max frequency
	scope->writeCmd(QString(":WGEN1:FREQ ") + QString::number(stop_freq));
	frequencies.append(stop_freq);
	double amplitude = scope->query(":MEASURE:VAMP?").toDouble();
	amplitudes.append(amplitude);
	double db = 20*log(amplitude/5);
	amplitudesDb.append(db);

//	for(int current_frequency = start_freq; current_frequency < stop_freq; current_frequency += step_size)
//	{
//		scope->writeCmd(QString(":TIMEBASE:RANGE ") + QString::number(double(4) / current_frequency));
//		scope->writeCmd(QString(":WGEN1:FREQ ") + QString::number(current_frequency));

//		double amplitude = scope->query(":MEASURE:VAMP?").toDouble();
//		amplitudes.append(amplitude);
//		frequencies.append(current_frequency);

//		if(amplitude > 0.5)
//			scope->writeCmd(QString(":CHANNEL1:RANGE ") + QString::number(1.5 * amplitude));
//		usleep(100000);
//	}

	if(ui->radioButtonYAmp->isChecked())
	{
		fraCurve->setSamples(frequencies, amplitudes);
		ui->plotFra->setAxisTitle(QwtPlot::yLeft, "Amplitude [V]");
	}
	else
	{
		fraCurve->setSamples(frequencies, amplitudesDb);
		ui->plotFra->setAxisTitle(QwtPlot::yLeft, "Amplitude [dB]");
	}
	ui->plotFra->replot();
}

void FraDialog::on_radioButtonXLog_toggled(bool checked)
{
	if(checked)
		ui->plotFra->setAxisScaleEngine(QwtPlot::xBottom, new QwtLogScaleEngine);
	else
		ui->plotFra->setAxisScaleEngine(QwtPlot::xBottom, new QwtLinearScaleEngine);
	int start_freq = ui->spinBoxStartFreq->value() * pow(1000, ui->frequencyStartMultiplier->currentIndex());
	int stop_freq = ui->spinBoxStopFreq->value() * pow(1000, ui->frequencyStopMultiplier->currentIndex());
	ui->plotFra->setAxisScale(QwtPlot::xBottom, start_freq, stop_freq);
	ui->plotFra->replot();
}

void FraDialog::on_radioButtonYAmp_toggled(bool checked)
{
	if(checked)
	{
		fraCurve->setSamples(frequencies, amplitudes);
		ui->plotFra->setAxisTitle(QwtPlot::yLeft, "Amplitude [V]");
	}
	else
	{
		fraCurve->setSamples(frequencies, amplitudesDb);
		ui->plotFra->setAxisTitle(QwtPlot::yLeft, "Amplitude [dB]");
	}
	ui->plotFra->replot();
}

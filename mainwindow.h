#pragma once

#include <QMainWindow>
#include <qwt/qwt_plot_curve.h>
#include <qwt/qwt_plot_panner.h>
#include <qwt/qwt_plot_marker.h>
#include <QCloseEvent>
#include <QThread>
#include "scope.h"

QT_BEGIN_NAMESPACE
namespace Ui { class MainWindow; }
QT_END_NAMESPACE

class MainWindow : public QMainWindow
{
	Q_OBJECT

public:
	MainWindow(QWidget *parent = nullptr);
	~MainWindow();

private:
	Ui::MainWindow *ui;
	Scope scope;
	QThread scopeThread;
	QwtPlotCurve *waveformCurve = nullptr;
	QwtPlotPanner *panner = nullptr;
	QwtPlotMarker *marker = nullptr;
	double timebaseRange = 0;
	double channelRange = 0;
	double channelOffset = 0;

	void closeEvent(QCloseEvent* event);
	bool autoconnect();

private slots:
	void plotWaveforms(MultiChannelWaveformData waveformData);
	void on_actionConnect_triggered();
	void on_actionInfo_triggered();
	void on_cmdQuery_clicked();
	void on_cmdSend_clicked();
	void setTimebaseRange(double range);
	void on_comboBoxReference_currentIndexChanged(int reference_mode);
	void on_cmdStop_clicked();
	void zoomTimebase(int amount);
	void zoomVertical(int amount);
	void updateChannelRange();
	void on_checkBoxACLF_stateChanged();
	void adjustXYAfterAutoscale(XYSettings autoscaleResult);
};

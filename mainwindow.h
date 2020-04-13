#pragma once

#include <QMainWindow>
#include <qwt/qwt_plot_curve.h>
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
	QwtPlotCurve* waveformCurve = nullptr;
	double timebaseRange = 0;

	void closeEvent(QCloseEvent* event);
	bool autoconnect();

private slots:
	void plotWaveform(WaveformPointsVector waveformData);
	void on_actionConnect_triggered();
	void on_actionInfo_triggered();
	void on_cmdQuery_clicked();
	void on_cmdSend_clicked();
	void setTimebaseRange(double range);
	void on_comboBoxReference_currentIndexChanged(int reference_mode);
	void on_cmdStop_clicked();
	void zoom(int amount);
};

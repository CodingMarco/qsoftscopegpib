#pragma once

#include "scope.h"
#include <QMainWindow>
#include <qwt/qwt_plot_curve.h>
#include <QCloseEvent>

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
	QwtPlotCurve* waveformCurve = nullptr;
	QTimer *wftimer = nullptr;

	void closeEvent(QCloseEvent* event);
	bool autoconnect();

private slots:
	void plotWaveform();
	void on_cmdStart_clicked();
	void on_cmdStop_clicked();
	void on_actionConnect_triggered();
	void on_actionInfo_triggered();
	void on_cmdQuery_clicked();
	void on_cmdSend_clicked();
	void on_cmdAutoscale_clicked();
	void on_comboBox_currentIndexChanged(const QString &points);
	void on_cmdZoomIn_clicked();
	void on_cmdZoomOut_clicked();
};

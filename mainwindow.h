#pragma once

#include "scope.h"
#include "instrumentConnection.h"
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
	InstrumentConnection* instrumentConnection = nullptr;
	Scope* scope = nullptr;
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
	void on_actionFequencyResponse_Analysis_triggered();
	void on_cmdSend_clicked();
};

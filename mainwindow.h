#pragma once

#include <QMainWindow>
#include "scope.h"
#include <qwt/qwt_plot_curve.h>

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
	Scope* scope;
	QwtPlotCurve* waveformCurve;
	QTimer *wftimer;

private slots:
	void plotWaveform();
	void on_cmdStart_clicked();
	void on_cmdConnect_clicked();
};

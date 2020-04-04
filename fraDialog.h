#pragma once

#include <QDialog>
#include <qwt_plot_curve.h>

#include "scope.h"

namespace Ui {
	class FraDialog;
}

class FraDialog : public QDialog
{
	Q_OBJECT

public:
	explicit FraDialog(Scope* m_scope, QWidget *parent = nullptr);
	~FraDialog();

private slots:
	void on_cmdStart_clicked();
	void on_radioButtonXLog_toggled(bool checked);

	void on_radioButtonYAmp_toggled(bool checked);

private:
	Ui::FraDialog *ui;
	Scope* scope;
	QwtPlotCurve* fraCurve;
	QVector<double> amplitudes;
	QVector<double> amplitudesDb;
	QVector<double> frequencies;
};

#pragma once

#include <QObject>
#include <qwt/qwt_plot_curve.h>

class CustomQwtPlotCurve : public QObject, public QwtPlotCurve
{
	Q_OBJECT

public:
	CustomQwtPlotCurve();

public slots:
	void setVisible(bool on) override;
};

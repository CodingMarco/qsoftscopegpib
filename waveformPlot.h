#pragma once

#include <qwt/qwt_plot.h>

#include <QWheelEvent>

class WaveformPlot : public QwtPlot
{
	Q_OBJECT

public:
	WaveformPlot(QWidget *parent);

protected:
	virtual void wheelEvent(QWheelEvent *event);

signals:
	void mouseScrolled(int amount);
};

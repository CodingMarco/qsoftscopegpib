#include "waveformPlot.h"

WaveformPlot::WaveformPlot(QWidget *parent)
{
	setCanvasBackground(QBrush(Qt::black));
}

void WaveformPlot::wheelEvent(QWheelEvent *event)
{
	emit mouseScrolled(event->angleDelta().y()/120);
}

#include "waveformPlot.h"
#include <QGuiApplication>

WaveformPlot::WaveformPlot(QWidget *parent)
{
	setCanvasBackground(QBrush(Qt::black));
}

void WaveformPlot::wheelEvent(QWheelEvent *event)
{
	if(QGuiApplication::queryKeyboardModifiers().testFlag(Qt::ShiftModifier))
		emit mouseWithShiftScrolled(event->angleDelta().y()/120);
	else
		emit mouseScrolled(event->angleDelta().y()/120);
}

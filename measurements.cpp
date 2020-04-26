#include "measurements.h"
#include "yin.h"

#include <QDebug>
#include <QElapsedTimer>

Measurements::Measurements(QObject *parent) : QObject(parent)
{

}

int Measurements::getFrequency(WaveformPointsVector& points, int sampleRate)
{
	Yin yin(sampleRate, points.length());

	int frequency = yin.getPitch(points);
	return frequency;
}

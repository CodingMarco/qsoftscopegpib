#include "measurements.h"
#include "yin.h"

#include <QDebug>
#include <QElapsedTimer>

#include <algorithm>

Measurements::Measurements(QObject *parent) : QObject(parent)
{

}

void Measurements::setWaveform(const WaveformPointsVector& m_points)
{
	this->points = m_points;
	yValues.resize(points.length());
	for(int i = 0; i < points.length(); i++)
	{
		yValues[i] = points[i].y();
	}
}

MeasurementResult Measurements::Frequency(int sampleRate)
{
	MeasurementResult result;
	Yin yin(sampleRate, points.length());

	result.value = yin.getPitch(points);

	return result;
}

MeasurementResult Measurements::Vmin()
{
	MeasurementResult result;

	int minValueIndex = std::distance(yValues.constBegin(), std::min_element(yValues.constBegin(), yValues.constEnd()));
	result.value = yValues[minValueIndex];

	return result;
}

MeasurementResult Measurements::Vmax()
{
	MeasurementResult result;

	int minValueIndex = std::distance(yValues.constBegin(), std::max_element(yValues.constBegin(), yValues.constEnd()));
	result.value = yValues[minValueIndex];

	return result;
}

MeasurementResult Measurements::Vavg()
{
	MeasurementResult result;

	double average = std::accumulate(yValues.constBegin(), yValues.constEnd(), 0.0) / yValues.size();
	result.value = average;

	return result;
}

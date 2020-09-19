#pragma once

#include <QObject>
#include <QVector>
#include "scopeNamespace.h"

struct MeasurementResult
{
	// Markers markers
	double value;
};

class Measurements : public QObject
{
	Q_OBJECT
public:
	explicit Measurements(QObject *parent = nullptr);
	void setWaveform(const WaveformPointsVector &m_points);
	MeasurementResult Frequency(int sampleRate);
	MeasurementResult Vmin();
	MeasurementResult Vmax();
	MeasurementResult Vavg();

private:
	WaveformPointsVector points;
	QVector<double> yValues;
};

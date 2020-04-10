#pragma once

#include <QObject>
#include <QVector>
#include <QMap>
#include <QPointF>
#include "scopeNamespace.h"
#include "gpibInstrument.h"

class Scope : public GpibInstrument
{
	Q_OBJECT

public:
	explicit Scope();

	// Get data
	QVector<QPointF> digitizeAndGetPoints();

	// Set parameters
	bool setPoints(POINTS m_points);
	bool setFormat(WAVEFORM_FORMAT m_format);
	bool setSourceChannel(int m_channel);
	bool setAcquireType(ACQUIRE_TYPE m_type);

	// Get parameters
	POINTS points() { return this->_points; }
	QString idn() { return query("*IDN?"); }

	// Misc
	void autoscale();
	bool digitize();

private:
	// Scope properties
	int _sourceChannel = 1;
	int _bytesPerPoint = -1;
	POINTS _points;
	WAVEFORM_FORMAT _format;
	ACQUIRE_TYPE _acquireType;

	// Misc
	bool _printCommands = false;

	// Read parameters and data from oscilloscope
	QMap<QString, double> getWaveformPreamble();
	QVector<ushort> getWaveformData();

signals:
	void timebaseRangeUpdated(double m_timebaseRange);

};

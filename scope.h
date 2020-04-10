#pragma once

#include <QObject>
#include <QVector>
#include <QMap>
#include "scopeNamespace.h"
#include "gpibInstrument.h"

class Scope : public GpibInstrument
{
	Q_OBJECT

public:
	explicit Scope();

	// Get data
	QVector<ushort> getWaveformData();

	// Set parameters
	bool setPoints(POINTS m_points);
	bool setFormat(WAVEFORM_FORMAT m_format);
	bool setSourceChannel(int m_channel);
	bool setAcquireType(ACQUIRE_TYPE m_type);

	// Get parameters
	POINTS points() { return this->_points; }
	double timebaseRange() { return this->_timebaseRange; }
	double channelRange() { return query("CHANNEL1:RANGE?").toDouble(); }
	QString idn() { return query("*IDN?"); }
	QMap<QString, double> waveformPreamble();

	// Misc
	void autoscale();
	bool digitize();

private:
	// Scope properties
	int _sourceChannel = 1;
	int _bytesPerPoint = -1;
	double _timebaseRange = -1;
	QVector<double> _channelsRange;
	POINTS _points;
	WAVEFORM_FORMAT _format;
	ACQUIRE_TYPE _acquireType;

	// Misc
	bool _printCommands = false;

	// Read / update parameters from oscilloscope
	double updateTimebaseRange();

signals:
	void timebaseRangeUpdated(double m_timebaseRange);

};

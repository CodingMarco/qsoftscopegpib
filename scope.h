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
	bool zoomIn();
	bool zoomOut();

	// Get parameters
	POINTS points() { return this->_points; }
	QString idn() { return query("*IDN?"); }
	double timebaseRange() { return this->_timebaseRange; }
	double channelRange(int channel) { return _channelsRange[channel]; }

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

	// Read parameters and data from oscilloscope
	QMap<QString, double> getWaveformPreamble();
	QVector<ushort> getWaveformData();
	double updateTimebaseRange();

	// Set horizontal and vertical parameters
	bool setTimebaseRange(double range);

signals:
	void timebaseRangeUpdated(double m_timebaseRange);

};

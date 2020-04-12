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
	bool setPoints(POINTS newPoints);
	bool setFormat(WAVEFORM_FORMAT m_format);
	bool setSourceChannel(int m_channel);
	bool setAcquireType(ACQUIRE_TYPE m_type);
	void setTimebaseReference(TIMEBASE_REFERENCE m_reference);
	bool zoomIn();
	bool zoomOut();

	// Get parameters
	POINTS points() { return this->_points; }
	QString idn() { return query("*IDN?"); }
	double timebaseRange() { return this->_points / this->_sampleRate; }
	double channelRange(int channel) { return _channelsRange[channel]; }
	TIMEBASE_REFERENCE timebaseReference() { return _timebaseReference; }

	// Misc
	void initializeParameters();
	void autoscale();
	bool digitize();

private:
	// Scope properties
	int _sourceChannel = 1;
	int _bytesPerPoint = -1;
	int _sampleRate = -1;
	double _timebaseRange = -1;
	QVector<double> _channelsRange;
	POINTS _points = POINTS(-1);
	WAVEFORM_FORMAT _format;
	ACQUIRE_TYPE _acquireType;
	TIMEBASE_REFERENCE _timebaseReference;

	// Misc
	bool _printCommands = false;

	// Read parameters and data from oscilloscope
	QMap<QString, double> getWaveformPreamble();
	QVector<ushort> getWaveformData();
	double updateTimebaseRange();
	int updateSampleRate();

	// Set horizontal and vertical parameters
	bool setTimebaseRange(double range);
	bool setSampleRate(int m_sampleRate);

signals:
	void timebaseRangeUpdated(double m_timebaseRange);

};

#pragma once

#include <QObject>
#include <QVector>
#include <QMap>
#include <QPointF>
#include <QTimer>
#include "scopeNamespace.h"
#include "gpibInstrument.h"

class Scope : public GpibInstrument
{
	Q_OBJECT

private:
	// Scope properties
	int _sourceChannel = 1;
	int _bytesPerPoint = -1;
	int _sampleRateIndex = -1;
	double _timebaseRange = -1;
	bool _waveformUpdateActive = false;
	QVector<double> _channelsRange;
	POINTS _points = POINTS(-1);
	WAVEFORM_FORMAT _format;
	ACQUIRE_TYPE _acquireType;
	TIMEBASE_REFERENCE _timebaseReference;
	QVector<int> validSampleRates
		= { int(2.0e9), int(1e9), int(500e6), int(250e6), int(100e6), int(50e6), int(25e6), int(10e6), int(5e6),
			int(2.5e6), int(1e6), int(500e3), int(250e3), int(100e3), int(50e3), int(25e3), int(10e3), int(5e3),
			int(2.5e3), int(1e3), int(500) };

	// Misc
	QTimer *waveformUpdateTimer;
	bool _printCommands = false;
	bool digitize();

	// Read parameters and data from oscilloscope
	QMap<QString, double> getWaveformPreamble();
	QVector<ushort> getWaveformData();
	double updateTimebaseRange();
	int updateSampleRate();

	// Set horizontal and vertical parameters
	bool setTimebaseRange(double range);
	bool setSampleRateByIndex(int m_sampleRate);

public:
	explicit Scope();

	// Get parameters
	POINTS points() { return this->_points; }
	QString idn() { return query("*IDN?"); }

	double maximumTimebaseRange() { return double(this->_points) / validSampleRates[_sampleRateIndex]; }

	double channelRange(int channel) { return _channelsRange[channel]; }
	TIMEBASE_REFERENCE timebaseReference() { return _timebaseReference; }
	bool waveformUpdateActive() { return _waveformUpdateActive; }

public slots:
	// Set parameters
	bool setPoints(QString newPoints);
	bool setPoints(POINTS newPoints);
	bool setFormat(WAVEFORM_FORMAT m_format);
	bool setSourceChannel(int m_channel);
	bool setAcquireType(ACQUIRE_TYPE m_type);
	void setTimebaseReference(TIMEBASE_REFERENCE m_reference);
	bool zoomIn();
	bool zoomOut();

	// Misc
	void autoscale();
	void startWaveformUpdate() { waveformUpdateTimer->start(); _waveformUpdateActive = true;  }
	void stopWaveformUpdate()  { waveformUpdateTimer->stop();  _waveformUpdateActive = false; }
	void singleWaveformUpdate();
	void initializeThreadRelatedStuff();

private slots:
	void digitizeAndGetPoints();

signals:
	void timebaseRangeUpdated(double m_timebaseRange);
	void waveformUpdated(WaveformPointsVector);
};

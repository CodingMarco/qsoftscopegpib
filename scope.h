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
	// Model properties
	// Models 545x2x have a maximum sample rate of 2 GSa/s on all channels simultaneously.
	// Models 545x0x are limited to 2 GSa/s with one channel on, 1 GSa/s with two channels on and 500 MSa/s with three or four channels on.
	const bool twoGigaSamplesOnAllChannels = true;
	// Scope properties
	int _bytesPerPoint = -1;
	int _sampleRateIndex = -1;
	bool _waveformUpdateActive = false;
	QVector<Channel> channels;
	POINTS _points = POINTS(-1);
	WAVEFORM_FORMAT _format = WAVEFORM_FORMAT_WORD;
	ACQUIRE_TYPE _acquireType;
	TIMEBASE_REFERENCE _timebaseReference;
	QVector<int> validSampleRates
		= { int(2.0e9), int(1e9), int(500e6), int(250e6), int(100e6), int(50e6), int(25e6), int(10e6), int(5e6),
			int(2.5e6), int(1e6), int(500e3), int(250e3), int(100e3), int(50e3), int(25e3), int(10e3), int(5e3),
			int(2.5e3), int(1e3), int(500) };

	int minimumSampleRateIndex();

	// Misc
	QTimer *waveformUpdateTimer;
	bool _printCommands = false;
	bool digitizeActiveChannels();
	WaveformPointsVector getPointsFromChannel(int channel);

	// Read parameters and data from oscilloscope
	QMap<QString, double> getWaveformPreamble();
	int updateSampleRateFromScope();

	// Set horizontal and vertical parameters
	bool setSampleRateByIndex(int m_sampleRate);

public:
	explicit Scope();

	// Get parameters
	POINTS points() { return this->_points; }
	QString idn() { return query("*IDN?"); }

	TIMEBASE_REFERENCE timebaseReference() { return _timebaseReference; }
	bool waveformUpdateActive() { return _waveformUpdateActive; }

public slots:
	// Set parameters
	bool setPoints(QString newPoints);
	bool setPoints(POINTS newPoints);
	bool setFormat(WAVEFORM_FORMAT m_format);
	bool setAcquireType(ACQUIRE_TYPE m_type);
	void setTimebaseReference(TIMEBASE_REFERENCE m_reference);
	void zoomIn(bool emitSignal = true);
	void zoomOut(bool emitSignal = true);

	// Get parameters
	double getChannelRange();
	double getChannelOffset();
	double nextLowerTimebaseRange();
	double optimalTimebaseRange();

	// Misc
	void initializeScope();
	void autoscale();
	void startWaveformUpdate() { waveformUpdateTimer->start(); _waveformUpdateActive = true;  }
	void stopWaveformUpdate()  { waveformUpdateTimer->stop();  _waveformUpdateActive = false; }
	void singleWaveformUpdate();
	void initializeThreadRelatedStuff();
	void toggleAcCouplingAndLfReject(bool toggle);
	void autoAdjustSampleRate(double newTimebaseRange);
	void setChannelRange(double m_channelRange);

	// Channels
	void setChannelEnabled(int channel, bool enable);

	// Trigger
	void setTriggerLevel(double level);
	void setTriggerHoldoff(double holdoff);
	void setTriggerSourceChannel(int channel);
	void enableTriggerNoiseReject(bool enable);

private slots:
	void digitizeAndGetPoints();

signals:
	void waveformUpdated(MultiChannelWaveformData, ScopeSettings);
	void autoscaleComplete(XYSettings autoscaleResult);
	void zoomed(double newOptimalTimebaseRange);
};

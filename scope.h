#pragma once

#include <QObject>
#include <QVector>
#include <QMap>
#include "scopeNamespace.h"
#include "instrumentConnection.h"

class Scope : public QObject
{
	Q_OBJECT
public:
	explicit Scope(InstrumentConnection *m_instrumentConnection, QObject *parent = nullptr);

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
	double channelRange() { return instrumentConnection->query("CHANNEL1:RANGE?").toDouble(); }
	QString idn() { return instrumentConnection->query("*IDN?"); }
	QMap<QString, double> waveformPreamble();

	// Misc
	void autoscale();
	bool writeCmd(QString cmd);
	QString query(QString cmd);

private:
	InstrumentConnection* instrumentConnection = nullptr;

	// Scope properties
	int _sourceChannel = -1;
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

	bool digitize();

signals:
	void timebaseRangeUpdated(double m_timebaseRange);

};

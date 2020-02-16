#pragma once

#include <QObject>
#include <gpib/ib.h>
#include <QVector>
#include "scopeNamespace.h"

class Scope : public QObject
{
	Q_OBJECT
public:
	explicit Scope(int addr, QObject *parent = nullptr);
	int writeCmd(QString cmd);
	int writeCmd(QString cmd, int param);

	// Get data
	QString readString();
	QVector<short> getWaveformData();

	// Set parameters
	bool setPoints(POINTS m_points);
	bool setFormat(WAVEFORM_FORMAT m_format);
	bool setSourceChannel(int m_channel);
	bool setAcquireType(ACQUIRE_TYPE m_type);

private:
	int device = -1;
	int sourceChannel = -1;
	int bytesPerPoint = -1;
	POINTS points;
	WAVEFORM_FORMAT format;
	ACQUIRE_TYPE acquireType;

	bool digitize();

signals:

};

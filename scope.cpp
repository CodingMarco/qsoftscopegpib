#include "scope.h"
#include <gpib/ib.h>
#include <QDebug>
#include <QByteArray>
#include <string.h>
#include <QElapsedTimer>
#include <QThread>
#include <unistd.h>
#include <QtEndian>
#include <QMessageBox>

Scope::Scope(QObject *parent) : QObject(parent)
{

	// Default values
//	writeCmd(":SYSTEM:HEADER OFF");
	this->setAcquireType(ACQUIRE_TYPE_NORMAL);
	this->setPoints(POINTS_512);
	this->setFormat(WAVEFORM_FORMAT_WORD);
	this->setSourceChannel(1);
	this->updateTimebaseRange();

//	QElapsedTimer t;
//	t.start();
//	printf("\r%Lims, %d", t.elapsed(), ibcnt);

}

QVector<ushort> Scope::getWaveformData()
{
	digitize();
//	writeCmd(":WAVEFORM:DATA?");

	ushort buffer[this->_points + 10/_bytesPerPoint];
	for(int i = 0; i < this->_points + 5; i++)
	{
		buffer[i] = 0x0;
	}
//	ibrd(_device, buffer, this->_points*this->_bytesPerPoint+(10/_bytesPerPoint));
	QVector<ushort> ret;
	ret.reserve(this->_points);
	for(int i = 10/_bytesPerPoint; i < this->_points+10/_bytesPerPoint; i++)
		ret.append(qToBigEndian(buffer[i]));

	ret[ret.length()-1] = ret[ret.length()-3];
	ret[ret.length()-2] = ret[ret.length()-3];
	return ret;
}

bool Scope::setPoints(POINTS m_points)
{
	this->_points = m_points;
//	writeCmd(QString(":ACQUIRE:POINTS "), m_points);
	return true;
}

bool Scope::setFormat(WAVEFORM_FORMAT m_format)
{
	this->_format = m_format;
	switch (this->_format) {
		case WAVEFORM_FORMAT_BYTE:
//			writeCmd(":WAVEFORM:FORMAT BYTE");
			this->_bytesPerPoint = 1;
			break;
		case WAVEFORM_FORMAT_WORD:
//			writeCmd(":WAVEFORM:FORMAT WORD");
			this->_bytesPerPoint = 2;
			break;
	}
	return true;
}

bool Scope::setSourceChannel(int m_channel)
{
	if(m_channel > 4 || m_channel < 1)
	{
		qWarning() << "Channel must be 1, 2, 3 or 4! Setting it to 1.";
		return false;
	}
	else
	{
		this->_sourceChannel = m_channel;
//		writeCmd(QString(":WAVEFORM:SOURCE CHANNEL"), this->_sourceChannel);
		return true;
	}
}

bool Scope::setAcquireType(ACQUIRE_TYPE m_type)
{
	this->_acquireType = m_type;
	switch (this->_acquireType) {
		case ACQUIRE_TYPE_RAW:
//			writeCmd(":ACQUIRE:TYPE RAW");
			break;
		case ACQUIRE_TYPE_NORMAL:
//			writeCmd(":ACQUIRE:TYPE NORMAL");
			break;
	}
	return true;
}

void Scope::autoscale()
{
//	this->writeCmd(":autoscale");
	updateTimebaseRange();
}

double Scope::updateTimebaseRange()
{
//	this->_timebaseRange = this->query("TIMEBASE:RANGE?").toDouble();
	emit timebaseRangeUpdated(this->_timebaseRange);
	return this->_timebaseRange;
}

bool Scope::digitize()
{
//	writeCmd(QString(":DIGITIZE CHANNEL"), this->_sourceChannel);
	return true;
}

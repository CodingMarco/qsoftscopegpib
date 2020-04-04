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

Scope::Scope(InstrumentConnection* m_instrumentConnection, QObject *parent)
	: QObject(parent), instrumentConnection(m_instrumentConnection)
{

	// Default values
	instrumentConnection->writeCmd(":SYSTEM:HEADER OFF");
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
	instrumentConnection->writeCmd(":WAVEFORM:DATA?");

	QByteArray data = instrumentConnection->readData(2 * this->_points);

	QVector<ushort> samples;
	samples.reserve(this->_points);

	for(int i = 0; i < this->_points; i++)
	{
		samples.append(*(((ushort*)data.data())+i));
	}
	return samples;
}

bool Scope::setPoints(POINTS m_points)
{
	_points = m_points;
	instrumentConnection->writeCmd(QString(":WAVEFORM:POINTS "), m_points);
	instrumentConnection->writeCmd(QString(":ACQUIRE:POINTS "), m_points);
	return true;
}

bool Scope::setFormat(WAVEFORM_FORMAT m_format)
{
	this->_format = m_format;
	switch (this->_format) {
		case WAVEFORM_FORMAT_BYTE:
			instrumentConnection->writeCmd(":WAVEFORM:FORMAT BYTE");
			this->_bytesPerPoint = 1;
			break;
		case WAVEFORM_FORMAT_WORD:
			instrumentConnection->writeCmd(":WAVEFORM:FORMAT WORD");
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
		instrumentConnection->writeCmd(QString(":WAVEFORM:SOURCE CHANNEL"), this->_sourceChannel);
		return true;
	}
}

bool Scope::setAcquireType(ACQUIRE_TYPE m_type)
{
	this->_acquireType = m_type;
	switch (this->_acquireType) {
		case ACQUIRE_TYPE_RAW:
			instrumentConnection->writeCmd(":ACQUIRE:TYPE RAW");
			break;
		case ACQUIRE_TYPE_NORMAL:
			instrumentConnection->writeCmd(":ACQUIRE:TYPE NORMAL");
			break;
	}
	return true;
}

QMap<QString, double> Scope::waveformPreamble()
{
	QMap<QString, double> preamble;
	QStringList params = instrumentConnection->query(":WAVEFORM:PREAMBLE?").split(',');
	preamble["xincrement"] = params[4].toDouble();
	preamble["xorigin"] = params[5].toDouble();
	preamble["xreference"] = params[6].toDouble();
	preamble["yincrement"] = params[7].toDouble();
	preamble["yorigin"] = params[8].toDouble();
	preamble["yreference"] = params[9].toDouble();
	return preamble;
}

void Scope::autoscale()
{
	instrumentConnection->writeCmd(":autoscale");
	updateTimebaseRange();
}

bool Scope::writeCmd(QString cmd)
{
	return instrumentConnection->writeCmd(cmd);
}

QString Scope::query(QString cmd)
{
	return instrumentConnection->query(cmd);
}

double Scope::updateTimebaseRange()
{
	this->_timebaseRange = instrumentConnection->query("TIMEBASE:RANGE?").toDouble();
	emit timebaseRangeUpdated(this->_timebaseRange);
	return this->_timebaseRange;
}

bool Scope::digitize()
{
	instrumentConnection->writeCmd(QString(":DIGITIZE CHANNEL"), this->_sourceChannel);
	return true;
}

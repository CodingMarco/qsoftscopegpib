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

volatile int ibsta;
volatile int iberr;
volatile int ibcnt;
volatile long ibcntl;

Scope::Scope(int addr, QObject *parent) : QObject(parent)
{
	// Connect to scope at specified address
	// /dev/gpib0, addr. 7, no second addr, 10s timeout, no EOI line, stop at EOS-Byte
	int eosmode = int('\n');
	this->_device = ibdev(0, addr, 0, T3s, 0, 0);
	if(this->_device == -1)
	{
		qCritical() << "Could not connect to GPIB-adapter";
		exit(EXIT_FAILURE);
	}
	writeCmd("*IDN?");
	if(iberr == EBUS)
	{
		QMessageBox::critical(nullptr, "Connection timed out!", "Connection timed out!");
		exit(EXIT_FAILURE);
	}
	qDebug() << "IDN: " << readString();
	// Default values
	writeCmd(":SYSTEM:HEADER OFF");
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
	writeCmd(":WAVEFORM:DATA?");

	ushort buffer[this->_points + 10/_bytesPerPoint];
	for(int i = 0; i < this->_points + 5; i++)
	{
		buffer[i] = 0x0;
	}
	ibrd(_device, buffer, this->_points*this->_bytesPerPoint+(10/_bytesPerPoint));
	QVector<ushort> ret;
	ret.reserve(this->_points);
	for(int i = 10/_bytesPerPoint; i < this->_points+10/_bytesPerPoint; i++)
		ret.append(qToBigEndian(buffer[i]));

	ret[ret.length()-1] = ret[ret.length()-3];
	ret[ret.length()-2] = ret[ret.length()-3];
	return ret;
}

int Scope::writeCmd(QString cmd)
{
	if(!cmd.endsWith('\n'))
		cmd.append('\n');
	if(_printCommands)
		qDebug() << "CMD:" << cmd;
	int ret = ibwrt(_device, cmd.toStdString().c_str(), cmd.length());
	return ret;
}

int Scope::writeCmd(QString cmd, int param)
{
	cmd.append(QString::number(param));
	cmd.append('\n');
	int ret = writeCmd(cmd);
	return ret;
}

bool Scope::setPoints(POINTS m_points)
{
	this->_points = m_points;
	writeCmd(QString(":ACQUIRE:POINTS "), m_points);
	return true;
}

bool Scope::setFormat(WAVEFORM_FORMAT m_format)
{
	this->_format = m_format;
	switch (this->_format) {
		case WAVEFORM_FORMAT_BYTE:
			writeCmd(":WAVEFORM:FORMAT BYTE");
			this->_bytesPerPoint = 1;
			break;
		case WAVEFORM_FORMAT_WORD:
			writeCmd(":WAVEFORM:FORMAT WORD");
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
		writeCmd(QString(":WAVEFORM:SOURCE CHANNEL"), this->_sourceChannel);
		return true;
	}
}

bool Scope::setAcquireType(ACQUIRE_TYPE m_type)
{
	this->_acquireType = m_type;
	switch (this->_acquireType) {
		case ACQUIRE_TYPE_RAW:
			writeCmd(":ACQUIRE:TYPE RAW");
			break;
		case ACQUIRE_TYPE_NORMAL:
			writeCmd(":ACQUIRE:TYPE NORMAL");
			break;
	}
	return true;
}

void Scope::autoscale()
{
	this->writeCmd(":autoscale");
	updateTimebaseRange();
}

double Scope::updateTimebaseRange()
{
	this->_timebaseRange = this->query("TIMEBASE:RANGE?").toDouble();
	emit timebaseRangeUpdated(this->_timebaseRange);
	return this->_timebaseRange;
}

bool Scope::digitize()
{
	writeCmd(QString(":DIGITIZE CHANNEL"), this->_sourceChannel);
	return true;
}

QString Scope::readString()
{
	char buffer[1000];
	ibrd(_device, buffer, sizeof(buffer));
	QString ret = QString::fromLocal8Bit(buffer).split('\n')[0];
	return ret;
}

QString Scope::query(QString cmd)
{
	this->writeCmd(cmd);
	return this->readString();
}

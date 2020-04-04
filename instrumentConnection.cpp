#include "instrumentConnection.h"
#include <visa.h>
#include <QDebug>

InstrumentConnection::InstrumentConnection(QObject *parent) : QObject(parent)
{

}

InstrumentConnection::~InstrumentConnection()
{
	closeInstrument();
}

bool InstrumentConnection::connectToInstrument(QString m_visaAddr)
{
	this->connectionType = CONNECTION_TYPE_USB;
	return visaInstrument.openInstrument(m_visaAddr);
}

bool InstrumentConnection::connectToInstrument(int m_gpibAddr)
{
	this->connectionType = CONNECTION_TYPE_GPIB;
	return gpibInstrument.openInstrument(m_gpibAddr);
}

void InstrumentConnection::closeInstrument()
{
	if(this->connectionType == CONNECTION_TYPE_USB)
		visaInstrument.closeInstrument();
	// GPIB?
}

QStringList InstrumentConnection::getAvailableVisaInstruments()
{
	return visaInstrument.getAvailableInstruments();
}

bool InstrumentConnection::writeCmd(QString cmd)
{
	if(connectionType == CONNECTION_TYPE_USB)
	{
		if(visaInstrument.isOpen())
		{
			visaInstrument.writeCmd(cmd);
			return true;
		}
		else
		{
			qDebug() << "Error: VISA-Instrument not open";
			return false;
		}
	}
	else if(connectionType == CONNECTION_TYPE_GPIB)
	{
		return false;
	}
	else
	{
		qDebug() << "Unknown connection type.";
		return false;
	}
}

bool InstrumentConnection::writeCmd(QString cmd, int param)
{
	return this->writeCmd(cmd + param);
}

QString InstrumentConnection::readString()
{
	if(connectionType == CONNECTION_TYPE_USB)
	{
		if(visaInstrument.isOpen())
		{
			return visaInstrument.readString();
		}
		else
		{
			qDebug() << "Error: VISA-Instrument not open";
			return "";
		}
	}
	else if(connectionType == CONNECTION_TYPE_GPIB)
	{
		return "";
	}
	else
	{
		qDebug() << "Unknown connection type.";
		return "";
	}
}

QString InstrumentConnection::query(QString cmd)
{
	if(connectionType == CONNECTION_TYPE_USB)
	{
		if(visaInstrument.isOpen())
		{
			return visaInstrument.query(cmd);
		}
		else
		{
			qDebug() << "Error: VISA-Instrument not open";
			return "";
		}
	}
	else if(connectionType == CONNECTION_TYPE_GPIB)
	{
		return "";
	}
	else
	{
		qDebug() << "Unknown connection type.";
		return "";
	}
}

QString InstrumentConnection::query(QString cmd, int param)
{
	return this->query(cmd + param);
}

QByteArray InstrumentConnection::readData(int bytesToRead)
{
	if(connectionType == CONNECTION_TYPE_USB)
		return visaInstrument.readData(bytesToRead);
	else
		return QByteArray();
}

QVector<ushort> InstrumentConnection::readWordData()
{
	return visaInstrument.readWordData();
}

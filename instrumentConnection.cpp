#include "instrumentConnection.h"
#include <visa.h>
#include <QDebug>

InstrumentConnection::InstrumentConnection(CONNECTION_TYPE m_connectionType, QObject *parent) : QObject(parent)
{
	this->connectionType = m_connectionType;
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

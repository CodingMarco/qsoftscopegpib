#include "visaInstrument.h"
#include <QDebug>

void VisaInstrument::printVisaError(QString errorMessage)
{
	char errorDescription[500];
	memset(errorDescription, 0, sizeof (errorDescription));
	viStatusDesc(instr, status, errorDescription);
	qDebug() << errorMessage << " with " << hex << status << ": " << QString(errorDescription);
}

VisaInstrument::VisaInstrument(QObject *parent) : QObject(parent)
{
	status = viOpenDefaultRM(&defaultRM);

	if (status < VI_SUCCESS)
	{
		printVisaError("Opening DefaultRM failed");
	}
}

VisaInstrument::~VisaInstrument()
{
	if(instr != -1)
		viClose(instr);
	if(defaultRM != -1)
		viClose(defaultRM);
	instr = -1;
	defaultRM = -1;
}

QString VisaInstrument::getLastVisaErrorString()
{
	char errorDescription[500];
	memset(errorDescription, 0, sizeof (errorDescription));
	viStatusDesc(instr, status, errorDescription);
	return QString("Error ")
			+ QString("0x%1").arg(status, 8, 16, QLatin1Char('0'))
			+ ": "
			+ QString(errorDescription);
}

QStringList VisaInstrument::getAvailableInstruments()
{
	QStringList availableInstruments;

	ViFindList findList;
	char instrDesc[100];
	ViString searchExpression = (char*)"USB?*";

	status = viFindRsrc(defaultRM, searchExpression, &findList, &retCount, instrDesc);
	if (status < VI_SUCCESS)
		printVisaError("Finding first instrument failed");
	else
		availableInstruments.append(QString(instrDesc));
	for(int i = 0; i < retCount-1; i++)
	{
		status = viFindNext(findList, instrDesc);
		if (status < VI_SUCCESS)
			printVisaError("Finding instrument failed");
		else
			availableInstruments.append(QString(instrDesc));
	}
	return availableInstruments;
}

bool VisaInstrument::openInstrument(QString m_visaAddress)
{
	qDebug() << "Opening instrument " << m_visaAddress;
	char c_addr[100];
	strcpy(c_addr, m_visaAddress.toStdString().c_str());

	status = viOpen(defaultRM, c_addr, VI_NULL, VI_NULL, &instr);
	if (status < VI_SUCCESS)
	{
		printVisaError(QString("Opening instrument ") + m_visaAddress + "failed");
		return false;
	}
	else
		return true;
}

bool VisaInstrument::isOpen()
{
	return instr != -1;
}

bool VisaInstrument::writeCmd(QString cmd)
{
	status = viWrite(instr, (ViByte*)cmd.toStdString().c_str(), cmd.length(), &retCount);
	if (status < VI_SUCCESS)
	{
		printVisaError("Writing cmd failed");
		return false;
	}
	else
		return true;
}

QString VisaInstrument::readString()
{
	char readbuf[1000];
	memset(readbuf, 0, 1000);
	status = viRead(instr, (ViPBuf)readbuf, 1000, &retCount);
	if (status < VI_SUCCESS)
	{
		printf("Read failed with %x\n", status);
		return "";
	}
	return QString(readbuf);
}

QString VisaInstrument::query(QString cmd)
{
	if(writeCmd(cmd))
		return readString();
	else
		return "";
}

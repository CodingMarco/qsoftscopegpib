#include "visaInstrument.h"
#include "scopeNamespace.h"
#include <QDebug>
#include <QtEndian>
#include <QString>

void VisaInstrument::printVisaError(QString errorMessage)
{
	char errorDescription[500];
	memset(errorDescription, 0, sizeof (errorDescription));
	viStatusDesc(instr, status, errorDescription);
	qDebug() << errorMessage.toStdString().c_str() << "with" << hex << status << ":" << QString(errorDescription);
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
	closeInstrument();
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
	if(status == VI_ERROR_RSRC_NFOUND)
		qDebug() << "Error: No instruments found!";
	else if (status < VI_SUCCESS)
		printVisaError("Finding first instrument failed");
	else
		availableInstruments.append(QString(instrDesc));
	if(retCount > 0)
	{
		for(int i = 0; i < retCount-1; i++)
		{
			status = viFindNext(findList, instrDesc);
			if (status < VI_SUCCESS)
				printVisaError("Finding instrument failed");
			else
				availableInstruments.append(QString(instrDesc));
		}
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
		printVisaError(QString("Opening instrument ") + m_visaAddress + "failed with " + getLastVisaErrorString());
		return false;
	}
	else
	{
		viSetAttribute(instr, VI_ATTR_TMO_VALUE, 10000);
		return true;
	}
}

void VisaInstrument::closeInstrument()
{
	if(instr != -1)
		viClose(instr);
	if(defaultRM != -1)
		viClose(defaultRM);
	instr = -1;
	defaultRM = -1;
	printf("Instrument closed!\n");
}

bool VisaInstrument::isOpen()
{
	return instr != -1;
}

bool VisaInstrument::writeCmd(QString cmd)
{
	if(!cmd.endsWith('\n'))
		cmd.append('\n');
	status = viWrite(instr, (ViByte*)cmd.toStdString().c_str(), cmd.length(), &retCount);
	if (status < VI_SUCCESS)
	{
		printVisaError("Writing cmd failed");
		return false;
	}
	else
		return true;
}

bool VisaInstrument::writeCmd(QString cmd, int param)
{
	if(cmd.endsWith('\n'))
		cmd.remove('\n');
	cmd.append(QString::number(param));
	cmd.append('\n');
	return writeCmd(cmd);
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

QByteArray VisaInstrument::readData(int bytesToRead)
{
	bytesToRead += 10; // For Header
	unsigned char buffer[bytesToRead];
	memset(buffer, 0, bytesToRead);

	viRead(instr, buffer, bytesToRead, &retCount);

	QByteArray ret;
	ret.reserve(retCount);
	for(int i = 10; i < retCount; i++)
		ret.append(qToBigEndian(buffer[i]));

	return ret;
}

QVector<ushort> VisaInstrument::readWordData()
{
	byte blockDataCount[2]; // For example {'#', '8'}
	viRead(instr, blockDataCount, 2, &retCount);
	if(retCount != 2 || blockDataCount[0] != '#')
		qDebug() << "Failed to read number of bytes of block data";

	int blockDataByteCount = blockDataCount[1] - '0'; // convert char to int
	char numberOfDataBytesToBeTransmitted[blockDataByteCount];
	viRead(instr, (ViPBuf)numberOfDataBytesToBeTransmitted, blockDataByteCount, &retCount);
	if(retCount != blockDataByteCount)
		qDebug() << "Failed to read block data";

	int nr = QString(numberOfDataBytesToBeTransmitted).toInt();

	QVector<ushort> wordData;
	wordData.reserve(nr);
	return wordData;
}

QString VisaInstrument::query(QString cmd)
{
	if(writeCmd(cmd))
		return readString();
	else
		return "";
}

QString VisaInstrument::query(QString cmd, int param)
{
	if(writeCmd(cmd, param))
		return readString();
	else
		return "";
}

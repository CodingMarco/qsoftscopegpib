#include "gpibInstrument.h"
#include <unistd.h>
#include <gpib/ib.h>
#include <QDebug>
#include <QMessageBox>
#include <QtEndian>

GpibInstrument::GpibInstrument(QObject *parent) : QObject(parent)
{

}

GpibInstrument::~GpibInstrument()
{

}

bool GpibInstrument::openInstrument(int addr)
{
	// Connect to scope at specified address
	// /dev/gpib0, addr. 7, no second addr, 10s timeout, no EOI line, stop at EOS-Byte
	this->instr = ibdev(0, addr, 0, T3s, 0, 0);
	if(this->instr == -1)
	{
		qCritical() << "Could not connect to GPIB-adapter";
		exit(EXIT_FAILURE);
	}
	QString idn = query("*IDN?");
	if(iberr == EBUS)
	{
		QMessageBox::critical(nullptr, "Connection timed out!", "Connection timed out!");
		status = iberr;
		return false;
	}
	else
	{
		qDebug() << "Opened GPIB instrument with IDN: " << idn;
		return true;
	}

}

bool GpibInstrument::closeInstrument()
{
	if(this->isOpen())
	{
		ibonl(instr, 0);
		instr = -1;
		return true;
	}
	return false;
}

bool GpibInstrument::isOpen()
{
	return this->instr != -1;
}

bool GpibInstrument::writeCmd(QString cmd)
{
	if(!cmd.endsWith('\n'))
		cmd.append('\n');
	status = ibwrt(instr, cmd.toStdString().c_str(), cmd.length());
	return true;
}

bool GpibInstrument::writeCmd(QString cmd, int param)
{
	if(cmd.endsWith('\n'))
		cmd.remove('\n');
	cmd.append(QString::number(param));
	cmd.append('\n');
	return writeCmd(cmd);
}

QString GpibInstrument::readString()
{
	char buffer[1000];
	memset(buffer, 0, sizeof(buffer));
	status = ibrd(instr, buffer, sizeof(buffer));
	QString ret = QString::fromLocal8Bit(buffer).split('\n')[0];
	return ret;
}

QString GpibInstrument::query(QString cmd)
{
	if(this->writeCmd(cmd))
		return this->readString();
	else
		return "";
}

QString GpibInstrument::query(QString cmd, int param)
{
	if(this->writeCmd(cmd, param))
		return this->readString();
	else
		return "";
}

int GpibInstrument::parseBlockData()
{
	char blockData[10];
	memset(blockData, 0, sizeof(blockData));
	status = ibrd(instr, blockData, sizeof(blockData));

	if(status < 0)
		qDebug() << "Failed to read block data";

	int i = 2;
	while(i < 8)
	{
		if(blockData[i] != '0')
			break;
		i++;
	}

	char dataBytes[10-i+1];
	memcpy(dataBytes, blockData+i, 10-i);
	dataBytes[10-i] = '\0';

	QString nr(dataBytes);
	int nrint = nr.toInt();
	return nrint;
}

QByteArray GpibInstrument::readBytes(int bytesToRead)
{
	QByteArray data;
	data.reserve(bytesToRead);
	data.resize(bytesToRead);
	data.fill(0);
	status = ibrd(instr, data.data(), bytesToRead);
	return data;
}

QByteArray GpibInstrument::readAllByteData()
{
	int bytesToRead = parseBlockData()+2;
	return readBytes(bytesToRead);
}

QVector<ushort> GpibInstrument::readAllWordData()
{
	return bytesToWord(readAllByteData());
}

QVector<ushort> GpibInstrument::bytesToWord(QByteArray bytes)
{
	QVector<ushort> wordData;
	wordData.reserve(bytes.size()/2);

	for(int i = 0; i < bytes.size()/2; i++)
	{
		wordData.append(qToBigEndian(*(((ushort*)bytes.data())+i)));
	}
	return wordData;
}

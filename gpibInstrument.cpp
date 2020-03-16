#include "gpibInstrument.h"
#include <gpib/ib.h>
#include <QDebug>
#include <QMessageBox>

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
	writeCmd("*IDN?");
	if(iberr == EBUS)
	{
		QMessageBox::critical(nullptr, "Connection timed out!", "Connection timed out!");
		status = iberr;
		return false;
	}
	else
	{
		qDebug() << "Opened GPIB instrument with IDN: " << readString();
		return true;
	}

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

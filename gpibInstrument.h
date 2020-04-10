#pragma once

#include <QObject>
#include <gpib/ib.h>

class GpibInstrument : public QObject
{
	Q_OBJECT

private:
	volatile int ibsta;
	volatile int iberr;
	volatile int ibcnt;
	volatile long ibcntl;

	int instr = -1;
	int status = -1;

	QVector<ushort> bytesToWord(QByteArray bytes);

public:
	explicit GpibInstrument(QObject *parent = nullptr);
	virtual ~GpibInstrument();
	bool openInstrument(int addr);
	bool closeInstrument();
	bool isOpen();
	bool writeCmd(QString cmd);
	bool writeCmd(QString cmd, int param);
	QString readString();
	QString query(QString cmd);
	QString query(QString cmd, int param);

	int parseBlockData();
	QByteArray readBytes(int bytesToRead);
	QByteArray readAllByteData();
	QVector<ushort> readAllWordData();

	void printSystErr();

protected:

};

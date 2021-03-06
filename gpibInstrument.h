#pragma once

#include <QObject>
#include <gpib/ib.h>

class GpibInstrument : public QObject
{
	Q_OBJECT

private:
	volatile int ibsta;
	volatile int iberr = 0;
	volatile int ibcnt;
	volatile long ibcntl;

	int instr = -1;
	int status = -1;
	int board_index = 0;

	QVector<ushort> bytesToWord(QByteArray bytes);

protected:
	int parseBlockData();
	QByteArray readBytes(int bytesToRead);
	QByteArray readAllByteData();
	QVector<ushort> readAllWordData();

public:
	explicit GpibInstrument(QObject *parent = nullptr);
	virtual ~GpibInstrument();
	bool openInstrument(int addr);
	bool closeInstrument();
	bool isOpen();
	void printSystErr();

public slots:
	bool writeCmd(QString cmd);
	bool writeCmd(QString cmd, int param);
	bool writeCmd(QString cmd, QString param);
	QString readString();
	QString query(QString cmd);
	QString query(QString cmd, int param);
};

#include "gpib.h"
#include <gpib/ib.h>
#include <QDebug>
#include <QByteArray>
#include <string.h>
#include <QElapsedTimer>
#include <QThread>
#include <unistd.h>

volatile int ibsta;
volatile int iberr;
volatile int ibcnt;
volatile long ibcntl;

gpib::gpib(QObject *parent) : QObject(parent)
{
	int device = openConnection(7);
	//writeCmd(":autoscale");
	writeCmd(":ACQUIRE:TYPE normal");
	writeCmd(":ACQUIRE:POINTS 2048");
	writeCmd(":WAVEFORM:FORMAT word");
	writeCmd(":WAVEFORM:SOURCE CHANNEL1");

	QElapsedTimer t;
	t.start();
	//qDebug() << "Bytes: " << ibcnt;
	for(int i = 0; i < 30; i++)
	{
		t.restart();
		//writeCmd(":WMEMORY1:GET CHANNEL1");
		writeCmd(":DIGITIZE CHANNEL1");
		writeCmd(":WAVEFORM:DATA?");
		QString str = readString();
		printf("\r%Lims, %d", t.elapsed(), ibcnt);
		printf("Last: %s", str.remove(50, str.size()-50).toStdString().c_str());
		fflush(stdout);
	}
	printf("\n");
}

int gpib::openConnection(int addr)
{
	// /dev/gpib0, addr. 7, no second addr, 10s timeout, no EOI line, stop at EOS-Byte
	this->device = ibdev(0, addr, 0, T10s, 0, 0);
	connected = true;
	return this->device;
}

int gpib::writeCmd(QString cmd)
{
	if(!cmd.endsWith('\n'))
		cmd.append('\n');
	int ret = ibwrt(device, cmd.toStdString().c_str(), cmd.length());
	return ret;
}

QVector<short> gpib::getWaveformData()
{
	writeCmd(":ACQUIRE:POINTS?");
	int points = readString().toInt();
	short buffer[points*2 + 11];
	ibrd(device, buffer, sizeof(buffer));
}

QString gpib::readString()
{
	char buffer[10000];
	ibrd(device, buffer, sizeof(buffer));
	QString ret = QString::fromLocal8Bit(buffer).split('\n')[0];
	return ret;
}

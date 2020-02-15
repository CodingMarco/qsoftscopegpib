#pragma once

#include <QObject>
#include <gpib/ib.h>
#include <QVector>

class gpib : public QObject
{
	Q_OBJECT
public:
	explicit gpib(QObject *parent = nullptr);

private:
	bool connected = false;
	int device = -1;
	int openConnection(int addr);
	int writeCmd(QString cmd);

	QVector<short> getWaveformData();

	QString readString();

signals:

};

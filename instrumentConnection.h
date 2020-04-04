#pragma once
#include <QObject>
#include <QString>
#include <QByteArray>
#include <QVector>
#include <gpib/ib.h>
#include "scopeNamespace.h"
#include "visaInstrument.h"
#include "gpibInstrument.h"

class InstrumentConnection : public QObject
{
	Q_OBJECT

private:
	CONNECTION_TYPE connectionType;
	VisaInstrument visaInstrument;
	GpibInstrument gpibInstrument;

public:
	explicit InstrumentConnection(QObject *parent = nullptr);
	virtual ~InstrumentConnection();
	bool connectToInstrument(QString m_visaAddr);
	bool connectToInstrument(int m_gpibAddr);
	void closeInstrument();
	QStringList getAvailableVisaInstruments();
	bool writeCmd(QString cmd);
	bool writeCmd(QString cmd, int param);
	QString readString();
	QString query(QString cmd);
	QString query(QString cmd, int param);
	QByteArray readData(int bytesToRead);
	QVector<ushort> readWordData();

signals:

};

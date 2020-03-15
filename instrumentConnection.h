#pragma once
#include <QObject>
#include <QString>
#include <gpib/ib.h>
#include "scopeNamespace.h"
#include "visaInstrument.h"

class InstrumentConnection : public QObject
{
	Q_OBJECT

private:
	CONNECTION_TYPE connectionType;
	VisaInstrument visaInstrument;

public:
	explicit InstrumentConnection(CONNECTION_TYPE m_connectionType, QObject *parent = nullptr);
	bool writeCmd(QString cmd);
	bool writeCmd(QString cmd, int param);
	QString readString();
	QString query(QString cmd);
	QString query(QString cmd, int param);

signals:

};

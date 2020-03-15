#pragma once

#include <QObject>
#include <QStringList>
#include <visa.h>
#include <visatype.h>

class VisaInstrument : public QObject
{
	Q_OBJECT

private:
	ViSession defaultRM = -1, instr = -1;
	QString visaAddress = "";

	ViStatus status;
	ViUInt32 retCount;

	void printVisaError(QString errorMessage);

public:
	explicit VisaInstrument(QObject *parent = nullptr);
	virtual ~VisaInstrument();
	QString getLastVisaErrorString();
	QStringList getAvailableInstruments();
	bool openInstrument(QString m_visaAddress);
	bool isOpen();
	bool writeCmd(QString cmd);
	QString readString();
	QString query(QString cmd);

signals:

};

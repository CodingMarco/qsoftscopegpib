#pragma once

#include <QObject>
#include <QStringList>
#include <QVector>
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
	void closeInstrument();
	bool isOpen();
	bool writeCmd(QString cmd);
	bool writeCmd(QString cmd, int param);
	QString readString();
	QByteArray readData(int bytesToRead);
	QVector<ushort> readWordData();
	QString query(QString cmd);
	QString query(QString cmd, int param);

signals:

};

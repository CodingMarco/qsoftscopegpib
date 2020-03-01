#pragma once
#include <QObject>

class InstrumentConnection : public QObject
{
	Q_OBJECT
public:
	explicit InstrumentConnection(QObject *parent = nullptr);

signals:

};

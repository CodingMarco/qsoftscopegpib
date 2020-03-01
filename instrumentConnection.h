#ifndef INSTRUMENTCONNECTION_H
#define INSTRUMENTCONNECTION_H

#include <QObject>

class instrumentConnection : public QObject
{
	Q_OBJECT
public:
	explicit instrumentConnection(QObject *parent = nullptr);

signals:

};

#endif // INSTRUMENTCONNECTION_H

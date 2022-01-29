#ifndef CONNECTIONWRAPPER_H
#define CONNECTIONWRAPPER_H

#include <QObject>

class ConnectionWrapper : public QObject
{
    Q_OBJECT
public:
    explicit ConnectionWrapper(QObject *parent = nullptr);

signals:

};

#endif // CONNECTIONWRAPPER_H

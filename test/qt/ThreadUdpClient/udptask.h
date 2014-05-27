#ifndef UDPTASK_H
#define UDPTASK_H

#include <QThread>


class UDPTask : public QThread
{
    Q_OBJECT
public:
    UDPTask();

signals:
    void signal_configure(int port);

public slots:
    void slot_doSomething();
    void slot_processLine(QString line);

protected:
    void run();
};

#endif // UDPTASK_H

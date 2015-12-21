#ifndef WORKER_H
#define WORKER_H
#include <QObject>

class Worker :  public QObject
{
      Q_OBJECT
public:
    Worker();
    volatile bool _started;

public slots:
    void slot_processLine(QString line);

};

#endif // WORKER_H

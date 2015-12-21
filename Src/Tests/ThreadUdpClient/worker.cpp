#include <QDebug>
#include <QThread>
#include "worker.h"
#include "Sleeper.h"

Worker::Worker()
    : _started(true)
{
}

void Worker::slot_processLine(QString line)
{
    QThread *currentThread = QThread::currentThread();
    if (_started) {
        qDebug() << currentThread->objectName();
        qDebug() << "Process Line:" << line;

        Sleeper::msleep(1000);
    }
}



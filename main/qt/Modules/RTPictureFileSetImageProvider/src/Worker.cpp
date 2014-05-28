#include <QDebug>
#include <QThread>
#include "worker.h"
#include "Sleeper.h"

Worker::Worker(RTPictureFileSetImageProvider *instance)
    :  _instance(instance)
{
}

void Worker::slot_processLine(QString line)
{
    _instance->processLine(line);
}



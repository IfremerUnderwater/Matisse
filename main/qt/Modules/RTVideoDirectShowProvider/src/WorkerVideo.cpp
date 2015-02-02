#include <QDebug>
#include <QThread>
#include "WorkerVideo.h"
#include "Sleeper.h"

WorkerVideo::WorkerVideo(RTVideoDirectShowProvider *instance)
    :  _instance(instance)
{
}

void WorkerVideo::slot_processLine(QString line)
{
    qDebug() << "Receive a line";
    _instance->dim2Received(line);
}


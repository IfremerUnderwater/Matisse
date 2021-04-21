#include <QDebug>
#include <QThread>
#include "WorkerVideo.h"
#include "Sleeper.h"

WorkerVideo::WorkerVideo(RTPVideoStreamProvider *instance)
    :  _instance(instance)
{
}

void WorkerVideo::slot_processLine(QString line)
{
    _instance->dim2Received(line);
}



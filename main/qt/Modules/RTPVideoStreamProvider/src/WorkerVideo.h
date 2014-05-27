#ifndef WORKERVIDEO_H
#define WORKERVIDEO_H
#include <QObject>
#include "RTPVideoStreamProvider.h"

class WorkerVideo :  public QObject
{
      Q_OBJECT
public:
    WorkerVideo(RTPVideoStreamProvider* instance);

public slots:
    void slot_processLine(QString line);

private:
    RTPVideoStreamProvider* _instance;

};

#endif // WORKER_H

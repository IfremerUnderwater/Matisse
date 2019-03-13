#ifndef WORKERVIDEO_H
#define WORKERVIDEO_H
#include <QObject>
#include "RTVideoDirectShowProvider.h"


class WorkerVideo :  public QObject
{
      Q_OBJECT
public:
    WorkerVideo(RTVideoDirectShowProvider* instance);

public slots:
    void slot_processLine(QString line);

private:
    RTVideoDirectShowProvider* _instance;

};

#endif // WORKER_H

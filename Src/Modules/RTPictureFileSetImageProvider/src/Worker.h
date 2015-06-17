#ifndef WORKER_H
#define WORKER_H
#include <QObject>
#include "RTPictureFileSetImageProvider.h"

class Worker :  public QObject
{
      Q_OBJECT
public:
    Worker(RTPictureFileSetImageProvider* instance);

public slots:
    void slot_processLine(QString line);
private:
    RTPictureFileSetImageProvider* _instance;

};

#endif // WORKER_H

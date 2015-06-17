#include <QDebug>


#include "udptask.h"
#include "dim2udplistener.h"

UDPTask::UDPTask()
{
}

void UDPTask::slot_doSomething()
{
    qDebug() << "Do something";
    qDebug() << QThread::currentThread()->objectName();
}

void UDPTask::slot_processLine(QString line)
{
    qDebug() << QThread::currentThread()->objectName();
    qDebug() << "Process Line:" << line;

    msleep(1000);
}




void UDPTask::run()
{
   qDebug() << "Run";
   qDebug() << QThread::currentThread()->objectName();

   Dim2UDPListener *dim2UDPListener = new Dim2UDPListener(this);
   dim2UDPListener->moveToThread(this);
   dim2UDPListener->slot_configure(7777);

   connect(dim2UDPListener, SIGNAL(signal_newline(QString)), this, SLOT(slot_processLine(QString)), Qt::QueuedConnection);
   //emit signal_configure(7777);

   // Enter event loop
   QThread::exec();
   qDebug() << "Sortie de boucle";

}

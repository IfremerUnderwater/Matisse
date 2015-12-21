#include "ProcessorWrapper.h"

ProcessorWrapper::ProcessorWrapper(QObject *parent) :
    QThread(parent)
{
}

int ProcessorWrapper::runProcessor(QList<QVariant> args)
{
    _args = args;
    start();
}

void ProcessorWrapper::run()
{
    _returnValue = runProcessor(_args);
    emit finished();

}

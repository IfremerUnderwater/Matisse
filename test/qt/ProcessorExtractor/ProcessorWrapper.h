#ifndef PROCESSORWRAPPER_H
#define PROCESSORWRAPPER_H

#include <QThread>
#include <QVariant>
#include <QList>

class ProcessorWrapper : public QThread
{
    Q_OBJECT
public:
    explicit ProcessorWrapper(QObject *parent = 0);
    int runProcessor(QList<QVariant> args);

protected:
    virtual void run();
    virtual int process(QList<QVariant>) = 0;

private:
    QList<QVariant> _args;
    int _returnValue;
    
signals:
    
public slots:
    
};

#endif // PROCESSORWRAPPER_H

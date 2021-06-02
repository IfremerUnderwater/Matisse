#include "processor.h"
#include <QDebug>

namespace matisse {

Processor::Processor(QObject *parent, QString name, QString comment, quint16 inNumber, quint16 outNumber) :
    QObject(parent),
    LifecycleComponent(name, "[MODULE " + name + "]: "),
    _comment(comment),
    _inNumber(inNumber),
    _outNumber(outNumber),
    _inputPortList(NULL),
    _outputPortList(NULL),
    _okStatus(true)
{
}

void Processor::postImage(quint32 port, Image &image)
{
    qDebug() << logPrefix() << "postImage on port" << port;

    if (!_outputPortList) {
        qWarning() << logPrefix() << "No connected ports! ";
        return;
    }

    bool found = false;
    foreach (ImageSetPort *imageSetPort, *_outputPortList ) {
        if (imageSetPort->portNumber == port) {
            imageSetPort->imageSet->addImage(&image);
            found = true;
            break;
        }
    }

    if (!found)
        qWarning() << logPrefix() << "Port " << port << " non connecté";
}

void Processor::flush(quint32 port)
{
    bool found = false;
    foreach (ImageSetPort *imageSetPort, *_outputPortList ) {
        if (imageSetPort->portNumber == port) {
            imageSetPort->imageSet->flush();
            found = true;
            break;
        }
    }
    if (!found)
        qWarning() << logPrefix() << "Port " << port << " non connecté";
}



bool Processor::setInputPortList(QList<ImageSetPort *> * inputPortList)
{
    _inputPortList = inputPortList;

    return true;
}

bool Processor::setOutputPortList(QList<ImageSetPort *> * outputPortList)
{
    _outputPortList = outputPortList;

    return true;
}

void Processor::fatalErrorExit(QString message)
{
      askToStop(true);
      emit signal_showErrorMessage(logPrefix(), message);
      _okStatus = false;
      emit signal_fatalError();
}

} // namespace matisse

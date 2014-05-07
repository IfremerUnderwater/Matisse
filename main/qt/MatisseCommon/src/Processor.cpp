#include "Processor.h"
#include <QDebug>

using namespace MatisseCommon;

Processor::Processor(QObject *parent, QString name, QString comment, quint16 inNumber, quint16 outNumber) :
    ImageListener(parent),
    _name(name),
    _comment(comment),
    _inNumber(inNumber),
    _outNumber(outNumber),
    _logPrefix("[MODULE " + _name + "]: "),
    _expectedParameters(QList<MatisseParameter>()),
    _inputPortList(NULL),
    _outputPortList(NULL),
    _context(NULL),
    _mosaicParameters(NULL)
{

}

void Processor::addExpectedParameter(QString structure, QString param)
{
    MatisseParameter parameter;
    parameter.structure = structure;
    parameter.param = param;
    _expectedParameters.append(parameter);

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

QString const &Processor::logPrefix() const
{
    return  _logPrefix;
}

QList<MatisseParameter> Processor::expectedParameters() {
    return _expectedParameters;
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



void Processor::callConfigure(Context * context, MatisseParameters * mosaicParameters)
{
    qDebug() << logPrefix() << "configure";
    _context = context;
    _mosaicParameters = mosaicParameters;
    configure(context, mosaicParameters);
}


void Processor::callStart()
{
    qDebug() << logPrefix() << "start";
    if (_outputPortList) {
        foreach (ImageSetPort *port, *_outputPortList) {
            qDebug() << logPrefix() << "  port de sortie connecte: " << port->portNumber;
        }
    }

    start();
}

void Processor::callStop()
{
    qDebug() << logPrefix() << "stop";
    stop();
}


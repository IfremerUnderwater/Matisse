#include "processor.h"
#include <QDebug>

namespace matisse {

Processor::Processor(QObject *_parent, QString _name, QString _comment, quint16 _in_number, quint16 _out_number) :
    QObject(_parent),
    LifecycleComponent(_name, "[MODULE " + _name + "]: "),
    m_comment(_comment),
    m_in_number(_in_number),
    m_out_number(_out_number),
    m_input_port_list(NULL),
    m_output_port_list(NULL),
    m_ok_status(true)
{
}

void Processor::postImage(quint32 _port, Image &_image)
{
    qDebug() << logPrefix() << "postImage on port" << _port;

    if (!m_output_port_list) {
        qWarning() << logPrefix() << "No connected ports! ";
        return;
    }

    bool found = false;
    foreach (ImageSetPort *image_set_port, *m_output_port_list ) {
        if (image_set_port->port_number == _port) {
            image_set_port->image_set->addImage(&_image);
            found = true;
            break;
        }
    }

    if (!found)
        qWarning() << logPrefix() << "Port " << _port << " non connecté";
}

void Processor::flush(quint32 _port)
{
    bool found = false;
    foreach (ImageSetPort *image_set_port, *m_output_port_list ) {
        if (image_set_port->port_number == _port) {
            image_set_port->image_set->flush();
            found = true;
            break;
        }
    }
    if (!found)
        qWarning() << logPrefix() << "Port " << _port << " non connecté";
}



bool Processor::setInputPortList(QList<ImageSetPort *> * _input_port_list)
{
    m_input_port_list = _input_port_list;

    return true;
}

bool Processor::setOutputPortList(QList<ImageSetPort *> * _output_port_list)
{
    m_output_port_list = _output_port_list;

    return true;
}

void Processor::fatalErrorExit(QString _message)
{
      askToStop(true);
      emit si_showErrorMessage(logPrefix(), _message);
      m_ok_status = false;
      emit si_fatalError();
}

} // namespace matisse

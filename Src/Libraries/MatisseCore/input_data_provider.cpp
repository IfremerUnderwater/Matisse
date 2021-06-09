#include "input_data_provider.h"

namespace matisse {

InputDataProvider::InputDataProvider(QObject *_parent, QString _name, QString _comment, quint16 _out_number) :
    QObject(_parent),
    LifecycleComponent(_name, "[SOURCE " + _name + "]: "),
    m_comment(_comment),
    m_out_number(_out_number)
{
}

InputDataProvider::~InputDataProvider()
{

}

QString InputDataProvider::comment() const
{
    return m_comment;
}

} // namespace matisse




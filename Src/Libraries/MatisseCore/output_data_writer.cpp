#include "output_data_writer.h"

namespace matisse {

OutputDataWriter::OutputDataWriter(QObject *parent, QString _name, QString _comment, quint16 _in_number) :
    QObject(parent),
    LifecycleComponent(_name, "[DESTINATION " + _name + "]: "),
    m_comment(_comment),
    m_in_number(_in_number),
    m_image_set(NULL)
{
}

OutputDataWriter::~OutputDataWriter()
{
}

ImageListenerOutput *OutputDataWriter::nextImageSet()
{
    return nullptr;
}

ImageSet *OutputDataWriter::imageSet() const
{
    return m_image_set;
}

void OutputDataWriter::setImageSet(ImageSet *_image_set)
{
    m_image_set = _image_set;
}

} // namespace matisse

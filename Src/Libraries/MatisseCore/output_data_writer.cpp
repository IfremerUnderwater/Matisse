#include "output_data_writer.h"

using namespace MatisseCommon;

OutputDataWriter::OutputDataWriter(QObject *parent, QString name, QString comment, quint16 inNumber) :
    QObject(parent),
    LifecycleComponent(name, "[DESTINATION " + name + "]: "),
    _comment(comment),
    _inNumber(inNumber),
    _imageSet(NULL)
{
}

OutputDataWriter::~OutputDataWriter()
{
}


ImageSet *OutputDataWriter::imageSet() const
{
    return _imageSet;
}

void OutputDataWriter::setImageSet(ImageSet *imageSet)
{
    _imageSet = imageSet;
}

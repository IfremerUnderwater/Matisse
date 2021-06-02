#ifndef ReconstructionLister_H
#define ReconstructionLister_H


#include "output_data_writer.h"
#include "picture_file_set.h"
#include "image_set.h"
#include "file_image.h"

using namespace matisse;

class ReconstructionLister : public OutputDataWriter
{
    Q_OBJECT
    Q_INTERFACES(matisse::OutputDataWriter)

#if QT_VERSION >= QT_VERSION_CHECK(5, 0, 0)
    Q_PLUGIN_METADATA(IID "ReconstructionLister")
#endif

public:
    explicit ReconstructionLister(QObject *parent = 0);
    virtual ~ReconstructionLister();

    virtual void onNewImage(quint32 port, Image &image);
    virtual void onFlush(quint32 port);
    virtual bool configure();
    virtual bool start();
    virtual bool stop();
    virtual QList<QFileInfo> rastersInfo();

private:
    QList<QFileInfo> _rastersInfo;

signals:
    
public slots:
    
};

#endif // ReconstructionLister_H

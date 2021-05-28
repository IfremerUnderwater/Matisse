#ifndef DTPICTUREFILESETIMAGEPROVIDER_H
#define DTPICTUREFILESETIMAGEPROVIDER_H


#include "input_data_provider.h"
#include "dim2_file_reader.h"
#include "picture_file_set.h"
#include "image_set.h"
#include "file_image.h"

using namespace MatisseCommon;

class DTPictureFileSetImageProvider : public InputDataProvider
{
    Q_OBJECT
    Q_INTERFACES(MatisseCommon::InputDataProvider)

#if QT_VERSION >= QT_VERSION_CHECK(5, 0, 0)
    Q_PLUGIN_METADATA(IID "DTPictureFileSetImageProvider")
#endif

public:
    explicit DTPictureFileSetImageProvider(QObject *parent = 0);
    virtual ~DTPictureFileSetImageProvider();

    virtual ImageSet * imageSet(quint16 port);
    virtual bool configure();
    virtual bool start();
    virtual bool stop();


private:
    PictureFileSet * _pictureFileSet;
    Dim2FileReader *_dim2FileReader;
    ImageSet * _imageSet;

signals:
    
public slots:
    
};

#endif // DTPICTUREFILESETIMAGEPROVIDER_H

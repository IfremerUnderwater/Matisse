#ifndef MATISSE_DT_PICTURE_FILE_SET_IMAGE_PROVIDER_H_
#define MATISSE_DT_PICTURE_FILE_SET_IMAGE_PROVIDER_H_


#include "input_data_provider.h"
#include "dim2_file_reader.h"
#include "picture_file_set.h"
#include "image_set.h"
#include "file_image.h"

namespace matisse {

class DTPictureFileSetImageProvider : public InputDataProvider
{
    Q_OBJECT
    Q_INTERFACES(matisse::InputDataProvider)

#if QT_VERSION >= QT_VERSION_CHECK(5, 0, 0)
    Q_PLUGIN_METADATA(IID "DTPictureFileSetImageProvider")
#endif

public:
    explicit DTPictureFileSetImageProvider(QObject *_parent = 0);
    virtual ~DTPictureFileSetImageProvider();

    virtual ImageSet * imageSet(quint16 _port);
    virtual bool configure();
    virtual bool start();
    virtual bool stop();


private:
    PictureFileSet * m_picture_file_set;
    nav_tools::Dim2FileReader *m_dim2_file_reader;
    ImageSet * m_image_set;

signals:
    
public slots:
    
};

} // namespace matisse

#endif // MATISSE_DT_PICTURE_FILE_SET_IMAGE_PROVIDER_H_

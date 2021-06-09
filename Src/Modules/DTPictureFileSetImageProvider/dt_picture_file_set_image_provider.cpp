#include <QDir>

#include "dt_picture_file_set_image_provider.h"
#include "file_utils.h"

#if QT_VERSION < QT_VERSION_CHECK(5, 0, 0)
Q_EXPORT_PLUGIN2(DTPictureFileSetImageProvider, DTPictureFileSetImageProvider)
#endif
using namespace nav_tools;
using namespace system_tools;

namespace matisse {

DTPictureFileSetImageProvider::DTPictureFileSetImageProvider(QObject *parent):
    InputDataProvider(NULL, "DTPictureFileSetImageProvider", "", 1),
    _pictureFileSet(NULL),
    _dim2FileReader(NULL)
{
    Q_UNUSED(parent)
    _imageSet = new ImageSet();
    addExpectedParameter("dataset_param", "dataset_dir");
    addExpectedParameter("dataset_param", "navFile");
    addExpectedParameter("algo_param", "First_processed_image");
    addExpectedParameter("algo_param", "Last_processed_image");
    addExpectedParameter("algo_param", "step_im");
    addExpectedParameter("algo_param", "scale_factor");

}

DTPictureFileSetImageProvider::~DTPictureFileSetImageProvider()
{

}



ImageSet * DTPictureFileSetImageProvider::imageSet(quint16 port)
{
    Q_UNUSED(port)
    return _imageSet;
}

bool DTPictureFileSetImageProvider::configure()
{
    qDebug() << logPrefix() << "DTPictureFileSetImageProvider configure";

    QString rootDirnameStr = m_matisse_parameters->getStringParamValue("dataset_param", "dataset_dir");

    bool isOk = false;
    QString navFileStr = m_matisse_parameters->getStringParamValue("dataset_param", "navFile");
    quint32 firstImageIndex = m_matisse_parameters->getIntParamValue("algo_param", "First_processed_image", isOk);
    if (!isOk) {
        firstImageIndex = 1;
    }
    quint32 lastImageIndex = m_matisse_parameters->getIntParamValue("algo_param", "Last_processed_image", isOk);
    if (!isOk) {
        lastImageIndex = InfInt;
    }
    quint32 stepIndex = m_matisse_parameters->getIntParamValue("algo_param", "step_im", isOk);
    if (!isOk) {
        stepIndex = 1;
    }

    /* Resolve UNIX paths ('~/...') for remote job execution */
    rootDirnameStr = FileUtils::resolveUnixPath(rootDirnameStr);
    navFileStr = FileUtils::resolveUnixPath(navFileStr);

    // TODO Améliorer le check
    if (rootDirnameStr.isEmpty() || navFileStr.isEmpty())
        return false;


    qDebug()<< "lastImageIndex: "  << lastImageIndex;
    qDebug()<< "rootDirnameStr: "  << rootDirnameStr;
    qDebug()<< "navFileStr: "  << navFileStr;

    QFileInfo fileInfo(rootDirnameStr);
    if (!fileInfo.exists()) {
        qDebug() << "Erreur rootDirName";
        return false;
    }

    fileInfo.setFile(navFileStr);
    // si le fichier est en absolu, on ignore l'aspect relatif...
    if (!fileInfo.isAbsolute()) {
        navFileStr.prepend(rootDirnameStr + QDir::separator());
    }
    fileInfo.setFile(navFileStr);

    if (!fileInfo.exists()) {
        qDebug() << "Erreur navFileStr" << fileInfo.absoluteFilePath();
        return false;
    }

    _pictureFileSet = new PictureFileSet(rootDirnameStr);
    // Le dive number est mis à 0 car lu dans le fichier...
    // on pourrait (normalement...) le lire dans le nom du répertoire...
    _dim2FileReader = new Dim2FileReader(navFileStr, firstImageIndex, lastImageIndex, stepIndex);


    qDebug() << "navFileStr, firstImageIndex, lastImageIndex, stepIndex:" << navFileStr << firstImageIndex << lastImageIndex << stepIndex;

    return true;

}

bool DTPictureFileSetImageProvider::start()
{
    qDebug() << logPrefix() << " inside start";

    bool ok;
    double scaleFactor = m_matisse_parameters->getDoubleParamValue("algo_param", "scale_factor", ok);

    emit si_processCompletion(0);
    emit si_userInformation("Building image set...");

    for(int i=0; i<_dim2FileReader->getNumberOfImages(); i++) {

         QString filename = _dim2FileReader->getImageFilename(i);
//         qDebug() << logPrefix() << " load image " << filename;
         QFileInfo fileInfo(_pictureFileSet ->rootDirname(), filename);

         if (fileInfo.exists() && fileInfo.isReadable()) {
             QString fileFormat = _dim2FileReader->getImageFormat(i);
             QString fileSource = _dim2FileReader->getImageSource(i);
             NavInfo navInfo = _dim2FileReader->getNavInfo(i);
             FileImage * newImage = new FileImage(_pictureFileSet, filename, fileSource, fileFormat, i, navInfo);
             newImage->setScaleFactor(scaleFactor);
             _imageSet->addImage(newImage);
         }

         double progressRatio = i/_dim2FileReader->getNumberOfImages();
         quint8 progress = progressRatio * 100;
         emit si_processCompletion(progress);
    }
    emit si_processCompletion(100);

    _imageSet->flush();

    qDebug() << logPrefix() << " out start";
    return true;
}

bool DTPictureFileSetImageProvider::stop()
{
    _imageSet->clear();
    return true;
}

} // namespace matisse


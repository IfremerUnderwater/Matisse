#include <QDir>

#include "DTPictureFileSetImageProvider.h"
Q_EXPORT_PLUGIN2(DTPictureFileSetImageProvider, DTPictureFileSetImageProvider)

DTPictureFileSetImageProvider::DTPictureFileSetImageProvider(QObject *parent):
    ImageProvider(NULL, "DTPictureFileSetImageProvider", "", 1),
    _pictureFileSet(NULL),
    _dim2FileReader(NULL)
{
    _imageSet = new ImageSet();
    addExpectedParameter("dataset_param", "dataset_dir");
    addExpectedParameter("dataset_param", "navFile");
}

DTPictureFileSetImageProvider::~DTPictureFileSetImageProvider()
{

}



ImageSet * DTPictureFileSetImageProvider::imageSet(quint16 port)
{
    return _imageSet;
}

bool DTPictureFileSetImageProvider::configure(Context *context, MatisseParameters *mosaicParameters)
{
    qDebug() << logPrefix() << "DTPictureFileSetImageProvider configure";
    QFileInfo* pXmlFileInfo=mosaicParameters->getXmlFileInfo();
    QString rootDirnameStr = mosaicParameters->getStringParamValue("dataset_param", "dataset_dir");

    bool isOk = false;
    QString navFileStr = mosaicParameters->getStringParamValue("dataset_param", "navFile");
    quint32 firstImageIndex = mosaicParameters->getIntParamValue("algo_param", "First_processed_image", isOk);
    if (!isOk) {
        firstImageIndex = 1;
    }
    quint32 lastImageIndex = mosaicParameters->getIntParamValue("algo_param", "Last_processed_image", isOk);
    if (!isOk) {
        lastImageIndex = InfInt;
    }
    quint32 stepIndex = mosaicParameters->getIntParamValue("algo_param", "step_im", isOk);
    if (!isOk) {
        stepIndex = 1;
    }

    // TODO Améliorer le check
    if (rootDirnameStr.isEmpty() || navFileStr.isEmpty() || pXmlFileInfo==NULL)
        return false;


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

    _pictureFileSet = new PictureFileSet(rootDirnameStr,false);
    // Le dive number est mis à 0 car lu dans le fichier...
    // on pourrait (normalement...) le lire dans le nom du répertoire...
    _dim2FileReader = new Dim2FileReader(navFileStr, firstImageIndex, lastImageIndex, stepIndex);


    qDebug() << "navFileStr, firstImageIndex, lastImageIndex, stepIndex:" << navFileStr << firstImageIndex << lastImageIndex << stepIndex;

    return true;

}

void DTPictureFileSetImageProvider::start()
{
    qDebug() << logPrefix() << " inside start";

    for(int i=0; i<_dim2FileReader->getNumberOfImages(); i++) {
         QString filename = _dim2FileReader->getImageFilename(i);
         qDebug() << logPrefix() << " load image " << filename;
         QFileInfo fileInfo(_pictureFileSet ->rootDirname(), filename);
         if (fileInfo.exists() && fileInfo.isReadable()) {
             QString fileFormat = _dim2FileReader->getImageFormat(i);
             QString fileSource = _dim2FileReader->getImageSource(i);
             NavInfo navInfo = _dim2FileReader->getNavInfo(i);
             FileImage * newImage = new FileImage(_pictureFileSet, filename, fileSource, fileFormat, i, navInfo);
             _imageSet->addImage(newImage);
         }

    }
    _imageSet->flush();

    qDebug() << logPrefix() << " out start";
}

void DTPictureFileSetImageProvider::stop()
{
//    if (_pictureFileSet)
//        delete _pictureFileSet;
//    if (_dim2FileReader)
//        delete _dim2FileReader;
    _imageSet->clear();
}


#include <QDir>

#include "DTPictureFileSetImageProvider.h"


DTPictureFileSetImageProvider::DTPictureFileSetImageProvider(QObject *parent):
    ImageProvider(parent), _pictureFileSet(NULL), _dim2FileReader(NULL)
{
    _imageSet = new ImageSet();
}

DTPictureFileSetImageProvider::~DTPictureFileSetImageProvider()
{
    if (_pictureFileSet)
        delete _pictureFileSet;
    if (_dim2FileReader)
        delete _dim2FileReader;
    delete _imageSet;
}



ImageSet * DTPictureFileSetImageProvider::getImageSet()
{
    return _imageSet;
}

bool DTPictureFileSetImageProvider::assemble(Context &context, MosaicParameters &mosaicParameters)
{

    QFileInfo* pXmlFileInfo=mosaicParameters.getXmlFileInfo();
    QString rootDirnameStr = mosaicParameters.getStringParamValue("dataset_param", "dataset_dir");

    bool isOk = false;
    bool isRelativeDir=mosaicParameters.getBoolParamValue("dataset_param", "relative_path", isOk);
    QString navFileStr = mosaicParameters.getStringParamValue("dataset_param", "navFile");

    // TODO Améliorer le check
    if (rootDirnameStr.isEmpty() || navFileStr.isEmpty() || pXmlFileInfo==NULL)
        return false;

    // TODO Use Absolute if path is relative?
    if (isRelativeDir) {
        rootDirnameStr = QDir::cleanPath( pXmlFileInfo->absolutePath() + QDir::separator() + rootDirnameStr);
        navFileStr = QDir::cleanPath( pXmlFileInfo->absolutePath() + QDir::separator() + navFileStr);
    }
    qDebug()<< "rootDirnameStr: "  << rootDirnameStr;
    qDebug()<< "navFileStr: "  << navFileStr;

    _pictureFileSet = new PictureFileSet(rootDirnameStr,false);
    _dim2FileReader = new Dim2FileReader(navFileStr);

    return true;


}

void DTPictureFileSetImageProvider::start()
{
    qDebug() << "DTPictureFileSetImageProvider Start";
    // remplissage de la liste à partir du contenu du fichier dim2tot
    if (_dim2FileReader->getNumberOfImages() == 0) {
        return;
    }

    for (int index = 0; index < _dim2FileReader->getNumberOfImages(); index++) {
        NavInfo navInfo = _dim2FileReader->getNavInfo(index);
        QString filename = _dim2FileReader->getImageFilename(index);

        // test existence image...
        QFileInfo fileInfo(_pictureFileSet ->getRootDirname(), filename);
        if (fileInfo.exists() && fileInfo.isReadable()) {
            FileImage * newImage = new FileImage(_pictureFileSet, filename, "Test", "", index, 0, navInfo, 0);
            _imageSet->addImage(newImage);
        }
    }
}

void DTPictureFileSetImageProvider::stop()
{
    qDebug() << "DTPictureFileSetImageProvider Stop";
}


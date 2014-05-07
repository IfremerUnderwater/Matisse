#include "FileImage.h"

using namespace MatisseCommon;



FileImage::FileImage(const FileImage &other):NavImage(other),
    _fileName(other._fileName),
    _sourceName(other._sourceName),
    _sourceFormat(other._sourceFormat),
    _pictureFileSet(other._pictureFileSet)
{
}

QString FileImage::dumpAttr()
{
    QString ret(NavImage::dumpAttr());
    ret += "fileName = " + _fileName + "\n";
    ret += "sourceName = " + _sourceName + "\n";
    ret += "sourceFormat = " + _sourceFormat + "\n";

    return ret;

}

Mat *FileImage::imageData() {

    if ( _imageData == 0) {
        // chargement de l'image
        // normalement elle existe car vérifiée dans le provider...
        if (!_pictureFileSet == 0) {
            std::string filePath =  QString(_pictureFileSet -> rootDirname() + "/" +_fileName).toStdString();

            _imageData = new Mat(imread(filePath));
        }
    }
    return _imageData;
}


FileImage::FileImage(PictureFileSet *pictureFileSet, QString fileName, QString sourceName, QString sourceFormat, int id, NavInfo navInfo):NavImage(id, NULL, navInfo),
    _fileName(fileName),
    _sourceName(sourceName),
    _sourceFormat(sourceFormat),
    _pictureFileSet(pictureFileSet)
{
}


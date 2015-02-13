#include "FileImage.h"

using namespace MatisseCommon;



FileImage::FileImage(const FileImage &other):NavImage(other),
    _fileName(other._fileName),
    _sourceName(other._sourceName),
    _sourceFormat(other._sourceFormat),
    _pictureFileSet(other._pictureFileSet)
{
    _imReader = new QImageReader(_pictureFileSet->rootDirname() + "/" +_fileName);
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

int FileImage::width()
{
    if(_imageData != 0){
        // If image is loaded send info from memory
        return _imageData->cols;
    }else{
        // If not send info from reader (no image loading)
        if(_imReader){
            return _imReader->size().width();
        }else{
            return -1;
        }
    }
}

int FileImage::height()
{
    if(_imageData != 0){
        // If image is loaded send info from memory
        return _imageData->rows;
    }else{
        // If not send info from reader (no image loading)
        if(_imReader){
            return _imReader->size().height();
        }else{
            return -1;
        }
    }
}

FileImage::FileImage(PictureFileSet *pictureFileSet, QString fileName, QString sourceName, QString sourceFormat, int id, NavInfo navInfo):NavImage(id, NULL, navInfo),
    _fileName(fileName),
    _sourceName(sourceName),
    _sourceFormat(sourceFormat),
    _pictureFileSet(pictureFileSet)
{
}


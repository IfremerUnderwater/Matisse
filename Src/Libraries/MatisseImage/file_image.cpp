#include "file_image.h"
#include <QDebug>

using namespace cv;
using namespace MatisseCommon;


FileImage::FileImage(PictureFileSet *pictureFileSet, QString fileName, QString sourceName, QString sourceFormat, int id, NavInfo navInfo):NavImage(id, NULL, navInfo),
    _fileName(fileName),
    _sourceName(sourceName),
    _sourceFormat(sourceFormat),
    _pictureFileSet(pictureFileSet)
{
    _imReader = new QImageReader(_pictureFileSet->rootDirname() + "/" +_fileName);
    _scaleFactor = 1.0;
}

FileImage::FileImage(const FileImage &other):NavImage(other),
    _fileName(other._fileName),
    _sourceName(other._sourceName),
    _sourceFormat(other._sourceFormat),
    _pictureFileSet(other._pictureFileSet)
{
    _imReader = new QImageReader(_pictureFileSet->rootDirname() + "/" +_fileName);
}

FileImage::~FileImage()
{
    delete _imReader;
}

QString FileImage::dumpAttr()
{
    QString ret(NavImage::dumpAttr());
    ret += "fileName = " + _fileName + "\n";
    ret += "sourceName = " + _sourceName + "\n";
    ret += "sourceFormat = " + _sourceFormat + "\n";

    return ret;

}

double FileImage::getScaleFactor() const
{
    return _scaleFactor;
}

void FileImage::setScaleFactor(double scaleFactor)
{
    _scaleFactor = scaleFactor;
}

Mat *FileImage::imageData() {

    if ( _imageData == 0) {
        // chargement de l'image
        // normalement elle existe car vérifiée dans le provider...
        if (!_pictureFileSet == 0) {
            std::string filePath =  QString(_pictureFileSet -> rootDirname() + "/" +_fileName).toStdString();

            if (_scaleFactor < 1.0){
                _imageData = new Mat();
                Mat fullSizeImg = imread(filePath, cv::IMREAD_COLOR | cv::IMREAD_IGNORE_ORIENTATION);
                resize(fullSizeImg, *_imageData, cv::Size(0, 0), _scaleFactor, _scaleFactor);
            }else{
                _imageData = new Mat(imread(filePath, cv::IMREAD_COLOR | cv::IMREAD_IGNORE_ORIENTATION));
            }
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
            return _scaleFactor*_imReader->size().width();
        }else{
            qDebug() << "Image Size cannot be read";
            exit(1);
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
            return _scaleFactor*_imReader->size().height();
        }else{
            qDebug() << "Image Size cannot be read";
            exit(1);
        }
    }
}


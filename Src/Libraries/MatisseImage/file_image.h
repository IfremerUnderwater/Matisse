#ifndef MATISSE_FILE_IMAGE_H_
#define MATISSE_FILE_IMAGE_H_



#include <QSharedPointer>

#include "nav_image.h"
#include "picture_file_set.h"
#include <QImageReader>


namespace matisse_image {
///
/// \brief The Image provenant d'un fichier
///
class FileImage : public NavImage
{
public:

    ///
    /// \brief Constructeur
    /// \param pictureFileSet référentiel fichier racine des images
    /// \param fileName nom de fichier de l'image
    /// \param sourceName nom de la source de l'image
    /// \param sourceFormat format de l'image
    /// \param id identifiant de l'image
    /// \param navInfo informations de navigation
    ///
    FileImage(PictureFileSet * pictureFileSet, QString fileName, QString sourceName, QString sourceFormat, int id, NavInfo navInfo);

    ///
    /// \brief Constructeur de copie
    /// \param other
    ///
    FileImage(const FileImage & other);

    ~FileImage();

    QString getFileName() {  return _fileName;  }
	
	QString getFullFilePath() {  return _pictureFileSet->rootDirname() + "/" +_fileName;  }

    QString getSourceName() { return _sourceName; }

    QString getSourceFormat() { return _sourceFormat;  }

    PictureFileSet * getPictureFileSet() { return _pictureFileSet;  }

    ///
    /// \brief Construction d'image Mat en lazy-loading
    /// \return
    ///
    virtual cv::Mat * imageData();

    ///
    /// \brief Return width of _imageData without loading image file
    /// \return width or -1 if not available
    ///
    virtual int width();
    ///
    /// \brief Return height of _imageData without loading image file
    /// \return height or -1 if not available
    ///
    virtual int height();


    ///
    /// \brief Fonction de deboggage
    /// \return
    ///
    virtual QString dumpAttr();

    double getScaleFactor() const;
    void setScaleFactor(double scaleFactor);

protected:
    QString _fileName;
    QString _sourceName;
    QString _sourceFormat;
    PictureFileSet * _pictureFileSet;
    QImageReader *_imReader;

    double _scaleFactor;

};

}

#endif // MATISSE_FILE_IMAGE_H_

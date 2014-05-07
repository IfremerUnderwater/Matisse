#ifndef FILEIMAGE_H
#define FILEIMAGE_H

#include <QSharedPointer>

#include "NavImage.h"
#include "PictureFileSet.h"

using namespace cv;

namespace MatisseCommon {
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

    QString getFileName() {  return _fileName;  }

    QString getSourceName() { return _sourceName; }

    QString getSourceFormat() { return _sourceFormat;  }

    PictureFileSet * getPictureFileSet() { return _pictureFileSet;  }

    ///
    /// \brief Construction d'image Mat en lazy-loading
    /// \return
    ///
    virtual Mat * imageData();

    ///
    /// \brief Fonction de deboggage
    /// \return
    ///
    virtual QString dumpAttr();

protected:
    QString _fileName;
    QString _sourceName;
    QString _sourceFormat;
    PictureFileSet * _pictureFileSet;

};

}

#endif // FILEIMAGE_H

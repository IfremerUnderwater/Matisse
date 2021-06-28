#ifndef MATISSE_IMAGE_FILE_IMAGE_H_
#define MATISSE_IMAGE_FILE_IMAGE_H_



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
    /// \param _picture_file_set référentiel fichier racine des images
    /// \param _file_name nom de fichier de l'image
    /// \param _source_name nom de la source de l'image
    /// \param _source_format format de l'image
    /// \param _id identifiant de l'image
    /// \param _nav_info informations de navigation
    ///
    FileImage(PictureFileSet * _picture_file_set, QString _file_name, QString _source_name, QString _source_format, int m_id, NavInfo _nav_info);

    ///
    /// \brief Constructeur de copie
    /// \param other
    ///
    FileImage(const FileImage & _other);

    ~FileImage();

    QString getFileName() {  return m_file_name;  }
	
    QString getFullFilePath() {  return m_picture_file_set->rootDirname() + "/" +m_file_name;  }

    QString getSourceName() { return m_source_name; }

    QString getSourceFormat() { return m_source_format;  }

    PictureFileSet * getPictureFileSet() { return m_picture_file_set;  }

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
    QString m_file_name;
    QString m_source_name;
    QString m_source_format;
    PictureFileSet * m_picture_file_set;
    QImageReader *m_im_reader;

    double m_scale_factor;

};

}

#endif // MATISSE_IMAGE_FILE_IMAGE_H_

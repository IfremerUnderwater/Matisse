#ifndef MATISSE_IMAGE_PICTURE_FILE_SET_H_
#define MATISSE_IMAGE_PICTURE_FILE_SET_H_


#include <QString>
#include <QDir>
#include <QFileInfo>
#include <QSharedPointer>

#include "input_source.h"

namespace matisse_image {

///
/// \brief Racine de l'arborescence d'une série d'image
///
class PictureFileSet : public InputSource
{
public:
    ///
    /// \brief Constructeur
    /// \param _root_dir_name chemin absolu vers la racine
    /// \param _dim2_file_name nom du fichier DIM2
    /// \param _writable mode d'accès aux fichiers images
    ///
    PictureFileSet(QString _root_dir_name = "", QString _dim2_file_name = "", bool _writable = false);


    QString rootDirname();
    QString dim2Filename();

    virtual bool isValid();
    ///
    /// \brief Retourne si le nom de fichier est valide
    /// \return
    ///
    bool rootDirnameIsValid();

    ///
    /// \brief Retourne si le fichier DIM2 est valide
    /// \return
    ///
    bool dim2FileIsValid();


private:
    QString m_root_dir_name;
    QString m_dim2_file_name;
    bool m_writable;
};
}
#endif // MATISSE_IMAGE_PICTURE_FILE_SET_H_

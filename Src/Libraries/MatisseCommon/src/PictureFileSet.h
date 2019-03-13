#ifndef PICTUREFILESET_H
#define PICTUREFILESET_H

#include "libmatissecommon_global.h"

#include <QString>
#include <QDir>
#include <QFileInfo>
#include <QSharedPointer>

#include "InputSource.h"
namespace MatisseCommon {

///
/// \brief Racine de l'arborescence d'une série d'image
///
class LIBMATISSECOMMONSHARED_EXPORT PictureFileSet : public InputSource
{
public:
    ///
    /// \brief Constructeur
    /// \param rootDirname chemin absolu vers la racine
    /// \param dim2Filename nom du fichier DIM2
    /// \param writable mode d'accès aux fichiers images
    ///
    PictureFileSet(QString rootDirname = "", QString dim2Filename = "", bool writable = false);


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
    QString _rootDirname;
    QString _dim2Filename;
    bool _writable;
};
}
#endif // PICTUREFILESET_H

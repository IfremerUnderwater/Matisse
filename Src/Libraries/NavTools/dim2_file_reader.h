#ifndef NAV_TOOLS_DIM2_FILE_READER_H_
#define NAV_TOOLS_DIM2_FILE_READER_H_



#include <QString>
#include <QHash>
#include <QFileInfo>
#include <QFile>
#include <QTextStream>
#include <QStringList>
#include <QRegExp>

#include <QtDebug>

#include "dim2.h"
#include "nav_info.h"

using namespace matisse_image;

namespace nav_tools {

///
/// \brief Classe de chargement de chargement de fichier DIM2.
///
class Dim2FileReader
{
public:
    ///
    /// \brief Constructeur. Appelle la methode readDim2File()
    /// \param filename le nom du fichier DIM2 à charger
    /// \param firstLine première ligne à lire
    /// \param lastLine dernière ligne à lire
    /// \param step pas entre lignes
    ///
    Dim2FileReader(QString filename = "", quint32 firstLine = 1, quint32 lastLine = 999999, quint32 step = 1);
    
    
    virtual ~Dim2FileReader();

    ///
    /// \brief Lit le fichier DIM2 et construit un NavInfo pour chaque ligne du fichier.
    /// \param firstLine première ligne à lire
    /// \param lastLine dernière ligne à lire
    /// \param step pas entre lignes
    /// \return true si le fichier a été lu correctement
    ///
    bool readDim2File(quint32 firstLine = 1, quint32 lastLine = 999999, quint32 step = 1);
   
    ///
    /// \brief Retourne le nom de l'image à l'index dans la collection des lignes lues
    /// \param imageId
    /// \return
    ///
    QString getImageFilename(quint32 imageId);

    ///
    /// \brief Retourne le nom de la source de l'image à l'index dans la collection des lignes lues
    /// \param imageId
    /// \return
    ///
    QString getImageSource(quint32 imageId);

    ///
    /// \brief Retourne le format de l'image à l'index dans la collection des lignes lues
    /// \param imageId
    /// \return
    ///
    QString getImageFormat(quint32 imageId);

    ///
    /// \brief Retourne les informations NavInfo correspondant à l'index dans la collection des lignes lues
    /// \param imageId
    /// \return
    ///
    NavInfo getNavInfo(quint32 imageId);

    int getNumberOfImages();

    ///
    /// \brief Retourne si le fichier a été lu correctement
    /// \return
    ///
    bool isFileValid();

    ///
    /// \brief Retourne le nom du fichier DIM2
    /// \return
    ///
    QString getFilename();

private:
    QString _filename;
    QList<Dim2> _dim2Lines;

    bool _fileIsValid;
    bool addDim2Line(QString datasLine);
};

}

#endif // NAV_TOOLS_DIM2_FILE_READER_H_

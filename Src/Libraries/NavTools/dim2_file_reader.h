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
    /// \param _filename le nom du fichier DIM2 à charger
    /// \param _first_line première ligne à lire
    /// \param _last_line dernière ligne à lire
    /// \param _step pas entre lignes
    ///
    Dim2FileReader(QString _filename = "", quint32 _first_line = 1, quint32 _last_line = 999999, quint32 _step = 1);
    
    
    virtual ~Dim2FileReader();

    ///
    /// \brief Lit le fichier DIM2 et construit un NavInfo pour chaque ligne du fichier.
    /// \param _first_line première ligne à lire
    /// \param _last_line dernière ligne à lire
    /// \param _step pas entre lignes
    /// \return true si le fichier a été lu correctement
    ///
    bool readDim2File(quint32 _first_line = 1, quint32 _last_line = 999999, quint32 _step = 1);
   
    ///
    /// \brief Retourne le nom de l'image à l'index dans la collection des lignes lues
    /// \param _image_id
    /// \return
    ///
    QString getImageFilename(quint32 _image_id);

    ///
    /// \brief Retourne le nom de la source de l'image à l'index dans la collection des lignes lues
    /// \param _image_id
    /// \return
    ///
    QString getImageSource(quint32 _image_id);

    ///
    /// \brief Retourne le format de l'image à l'index dans la collection des lignes lues
    /// \param _image_id
    /// \return
    ///
    QString getImageFormat(quint32 _image_id);

    ///
    /// \brief Retourne les informations NavInfo correspondant à l'index dans la collection des lignes lues
    /// \param _image_id
    /// \return
    ///
    NavInfo getNavInfo(quint32 _image_id);

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
    QString m_filename;
    QList<Dim2> m_dim2_lines;

    bool m_file_is_valid;
    bool addDim2Line(QString _data_line);
};

}

#endif // NAV_TOOLS_DIM2_FILE_READER_H_

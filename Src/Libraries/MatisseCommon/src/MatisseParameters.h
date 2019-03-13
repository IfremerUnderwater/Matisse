#ifndef MOSAICPARAMETERS_H
#define MOSAICPARAMETERS_H

#include "libmatissecommon_global.h"

#include <QtCore>
#include <QString>
#include <QMatrix3x3>
#include <QFile>
#include <QFileInfo>
#include <QXmlStreamReader>
#include <QXmlAttributes>
#include <QHash>
#include <QVariant>
#include <QRegExp>

#define InfInt qPow(2,31)-1
#define InfDouble exp((double)100)

namespace MatisseCommon {


//structures utilitaires
typedef QGenericMatrix<6, 1, double> Matrix6x1;

///
/// \brief Structure composite pour valeur de retour
///
class LIBMATISSECOMMONSHARED_EXPORT MatisseParameter {
public:
    QString structure;
    QString param;
};

///
/// \brief Classe d'acces aux parametres charges depuis un fichier XML
///
/// Cette classe permet d'accèder aux paramètres programmés par l'utilisateur ou l'expert depuis les instances de Processeurs ou celles d'ImageProvider.
/// es parametres sont accessibles en lecture seule.
///
class LIBMATISSECOMMONSHARED_EXPORT MatisseParameters
{
public:
    ///
    /// \brief Construit l'instance et charge le fichier XML si renseigne.
    ///
    /// \param xmlFilename chemin d'access au fichier.
    ///
    MatisseParameters(QString xmlFilename = "");
    virtual ~MatisseParameters();

    ///
    /// \brief Charge de fichier XML stocke les paramètres en mémoire
    /// \param xmlFilename Chemin d'access au fichier
    /// \return
    ///
    bool loadFile(QString xmlFilename);

    ///
    /// \brief Retourne l'information relative au fichier de paramètres
    /// \return
    ///
    QFileInfo* getXmlFileInfo();

    ///
    /// \brief Retourne si une erreur est détectée au chargement
    /// \return
    ///
    QString lastErrorStr();

    ///
    /// \brief dumpStructures pour debuggage
    /// \return
    ///
    QString dumpStructures();

    ///
    /// \brief Retourne vrai si le couple (paramStructName,paramName) a été chargé
    /// \param paramStructName Le groupe du paramètre
    /// \param paramName Le Nom du paramètre
    /// \return false si le paramètre n'existe pas
    ///
    bool containsParam(QString paramStructName, QString paramName);

    // Methodes d'acces aux parametres

    ///
    /// \brief Retourne en qint64 la valeur du couple (paramStructName,paramName)
    /// \param paramStructName
    /// \param paramName
    /// \param ok true si la valeur est convertible
    /// \return 2^31-1 si valeur="inf"
    ///
    qint64 getIntParamValue(QString paramStructName, QString paramName, bool &ok);

    ///
    /// \brief Retourne en bool la valeur du couple (paramStructName,paramName)
    /// \param paramStructName
    /// \param paramName
    /// \param ok true si la valeur est convertible
    /// \return true ou false
    ///
    bool getBoolParamValue(QString paramStructName, QString paramName, bool &ok);

    ///
    /// \brief Retourne en double la valeur du couple (paramStructName,paramName)
    /// \param paramStructName
    /// \param paramName
    /// \param ok true si la valeur est convertible
    /// \return 2^31-1 si valeur="inf"
    ///
    double getDoubleParamValue(QString paramStructName, QString paramName, bool &ok);

    ///
    /// \brief Retourne en QString la valeur du couple (paramStructName,paramName)
    /// \param paramStructName
    /// \param paramName
    /// \return
    ///
    QString getStringParamValue(QString paramStructName, QString paramName);

    ///
    /// \brief Retourne en QMatrix3x3 la valeur du couple (paramStructName,paramName)
    /// \param paramStructName
    /// \param paramName
    /// \param ok true si la valeur est convertible
    /// \return
    ///
    QMatrix3x3 getMatrix3x3ParamValue(QString paramStructName, QString paramName, bool &ok);

    ///
    /// \brief Retourne en Matrix6x1 la valeur du couple (paramStructName,paramName)
    /// \param paramStructName
    /// \param paramName
    /// \param ok true si la valeur est convertible
    /// \return
    ///
    Matrix6x1 getMatrix6x1ParamValue(QString paramStructName, QString paramName, bool &ok);

private:
    QString _lastErrorStr;
    // Structure de stockage des parametres
    QHash<QString, QHash<QString, QString> > _hashValues;
    QFileInfo * pFileInfo;
    static QRegExp _boolRegExpTrue;
    static QRegExp _boolRegExpFalse;
};
}

#endif // MOSAICPARAMETERS_H

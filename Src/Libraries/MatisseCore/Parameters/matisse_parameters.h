#ifndef MATISSE_MATISSE_PARAMETERS_H_
#define MATISSE_MATISSE_PARAMETERS_H_

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
#include "camera_info.h"

#define InfInt qPow(2,31)-1
#define InfDouble exp((double)100)

namespace matisse {


//structures utilitaires
typedef QGenericMatrix<6, 1, double> Matrix6x1;

///
/// \brief Structure composite pour valeur de retour
///
class MatisseParameter {
public:
    QString m_structure;
    QString m_param;
};

///
/// \brief Classe d'acces aux parametres charges depuis un fichier XML
///
/// Cette classe permet d'accèder aux paramètres programmés par l'utilisateur ou l'expert depuis les instances de Processeurs ou celles d'ImageProvider.
/// es parametres sont accessibles en lecture seule.
///
class MatisseParameters
{
public:
    ///
    /// \brief Construit l'instance et charge le fichier XML si renseigne.
    ///
    /// \param _xml_filename chemin d'access au fichier.
    ///
    MatisseParameters(QString _xml_filename = "");
    virtual ~MatisseParameters();

    ///
    /// \brief Charge de fichier XML stocke les paramètres en mémoire
    /// \param _xml_filename Chemin d'access au fichier
    /// \return
    ///
    bool loadFile(QString _xml_filename);

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
    /// \brief Retourne vrai si le couple (_param_struct_name,_param_name) a été chargé
    /// \param _param_struct_name Le groupe du paramètre
    /// \param _param_name Le Nom du paramètre
    /// \return false si le paramètre n'existe pas
    ///
    bool containsParam(QString _param_struct_name, QString _param_name);

    // Methodes d'acces aux parametres

    ///
    /// \brief Retourne en qint64 la valeur du couple (_param_struct_name,_param_name)
    /// \param _param_struct_name
    /// \param _param_name
    /// \param _ok true si la valeur est convertible
    /// \return 2^31-1 si valeur="inf"
    ///
    qint64 getIntParamValue(QString _param_struct_name, QString _param_name, bool &_ok);

    ///
    /// \brief Retourne en bool la valeur du couple (_param_struct_name,_param_name)
    /// \param _param_struct_name
    /// \param _param_name
    /// \param _ok true si la valeur est convertible
    /// \return true ou false
    ///
    bool getBoolParamValue(QString _param_struct_name, QString _param_name, bool &_ok);

    ///
    /// \brief Retourne en double la valeur du couple (_param_struct_name,_param_name)
    /// \param _param_struct_name
    /// \param _param_name
    /// \param _ok true si la valeur est convertible
    /// \return 2^31-1 si valeur="inf"
    ///
    double getDoubleParamValue(QString _param_struct_name, QString _param_name, bool &_ok);

    ///
    /// \brief Retourne en QString la valeur du couple (_param_struct_name,_param_name)
    /// \param _param_struct_name
    /// \param _param_name
    /// \return
    ///
    QString getStringParamValue(QString _param_struct_name, QString _param_name);

    ///
    /// \brief Retourne en QMatrix3x3 la valeur du couple (_param_struct_name,_param_name)
    /// \param _param_struct_name
    /// \param _param_name
    /// \param ok true si la valeur est convertible
    /// \return
    ///
    QMatrix3x3 getMatrix3x3ParamValue(QString _param_struct_name, QString _param_name, bool &_ok);

    ///
    /// \brief Retourne en Matrix6x1 la valeur du couple (_param_struct_name,_param_name)
    /// \param _param_struct_name
    /// \param _param_name
    /// \param ok true si la valeur est convertible
    /// \return
    ///
    Matrix6x1 getMatrix6x1ParamValue(QString _param_struct_name, QString _param_name, bool &_ok);

    CameraInfo getCamInfoParamValue(QString _param_struct_name, QString _param_name, bool &_ok);

private:
    QString m_last_error_str;
    // Structure de stockage des parametres
    QHash<QString, QHash<QString, QString> > m_hash_values;
    QFileInfo * m_p_file_info;
    static QRegExp m_bool_reg_exp_true;
    static QRegExp m_bool_reg_exp_false;
};

} // namespace matisse

#endif // MATISSE_MATISSE_PARAMETERS_H_

#ifndef LIFECYCLECOMPONENT_H
#define LIFECYCLECOMPONENT_H



#include <QList>
#include <QtPlugin>

#include "Context.h"
#include "matisse_parameters.h"

namespace MatisseCommon {
class LifecycleComponent
{
public:
    LifecycleComponent(QString name, QString logPrefix);
    virtual ~LifecycleComponent() {}

    ///
    /// \brief Retourne le nom unique spécifié dans le constructeur.
    /// \return
    ///
    QString name() const;


    ///
    /// \brief Appelle configure() de la classe dérivée
    ///
    /// Methode appelée par le moteur d'assemblage
    /// \param context Le contexte permet de passer des objets entre les différents processeurs.
    /// \param matisseParameters Les paramètres sont les informations saisies par l'utilisateur en lecture seule.
    ///
    bool callConfigure(Context * context, MatisseParameters * matisseParameters);

    ///
    /// \brief Appelle start() de la classe dérivée
    ///
    /// Methode appelée par le moteur d'assemblage
    bool callStart();

    ///
    /// \brief Demande au processeur de s'arrêter
    /// \param cancel demande l'annulation du travail
    ///
    bool askToStop(bool cancel=false);

    ///
    /// \brief Appelle stop() de la classe dérivée
    ///
    /// Methode appelée par le moteur d'assemblage
    bool callStop();


    ///
    /// \brief Retourne les le couple (structure,parametre) attendus dans MatisseParameters. Utilisé par le moteur d'assemblage.
    /// \return
    ///
    QList<MatisseParameter> expectedParameters() const;

protected:
    //
    // Methodes à surcharger - DEBUT
    //
    ///
    /// \brief Configuration du processeur pour l'execution d'un assemblage
    ///

    /// \return
    ///
    virtual bool configure() = 0;

    ///
    /// \brief La méthode start est appelée pour prevenir du commencement de l'execution d'un asssemblage.
    ///
    virtual bool start() = 0;

    ///
    /// \brief La méthode stop est appelée pour prevenir de la fin de l'execution d'un assemblage.
    ///
    virtual bool stop() = 0;


    //
    // Methodes à surcharger - FIN
    //

protected:
    ///
    /// \brief Ajoute un couple (structure,parametre) aux paramètres attendus. Doit être appelé dans le constructeur de la classe dérivée.
    /// \param structure
    /// \param param
    ///
    void addExpectedParameter(QString structure, QString param);

    /// <summary>
    /// Return an absolute path to input dataset dir
    /// </summary>
    /// <returns></returns>
    QString absoluteDatasetDir();

    /// <summary>
    /// Return absolute path to output dir
    /// </summary>
    /// <returns></returns>
    QString absoluteOutputDir();

    /// <summary>
    /// Return absolute path to Temp directory
    /// </summary>
    /// <returns></returns>
    QString absoluteOutputTempDir();

    ///
    /// \brief Retourne un chaine pour le déboggage
    /// \return
    ///
    QString logPrefix() const;

    ///
    /// \brief Retourne l'état d'execution du processeur
    ///
    /// Doit être appelé périodiquement pour vérifier si l'execution est interrompue.
    /// \return
    ///
    bool isStarted() const;

    ///
    /// \brief Retourne l'état d'arret du processeur
    ///
    /// \return
    ///
    bool isCancelled() const;


protected:
    Context * _context;
    MatisseParameters * _matisseParameters;

private:
    QString _name;
    QList<MatisseParameter> _expectedParameters;
    QString _logPrefix;

    volatile bool _isStarted;
    volatile bool _isCancelled;
};
}
Q_DECLARE_INTERFACE(MatisseCommon::LifecycleComponent, "Ifremer.LifecycleComponent/1.1")
#endif // LIFECYCLECOMPONENT_H

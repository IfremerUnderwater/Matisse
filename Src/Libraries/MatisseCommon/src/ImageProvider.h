#ifndef IMAGEPROVIDER_H
#define IMAGEPROVIDER_H


#include "libmatissecommon_global.h"

#include <QObject>
#include "ImageSet.h"
#include "ImageSet.h"
#include "Context.h"
#include "LifecycleComponent.h"
#include "Polygon.h"

#ifdef WIN32
#include <qgspoint.h>
#else
#include <qgis/qgspoint.h>
#endif

namespace MatisseCommon {
///
/// \brief Classe de base des fournisseurs d'image
/// Un fournisseur d'image possède plusieurs port de sortie d'image. Les ports de sortie sont connectés aux ports d'entrées des Processors par assemblage.
/// Une instance d'ImageProvider est construite au démarrage de Matisse.
/// Les methodes de cycle de vie virtuelles pures \ref configure(Context * , MatisseParameters *)\ref start() \ref stop() doivent être implémentées par les classes dérivées.
/// Elles sont appelées par le moteur à chaque exécution d'un assemblage.
///
class LIBMATISSECOMMONSHARED_EXPORT ImageProvider : public QObject, public LifecycleComponent
{
    Q_OBJECT
    Q_INTERFACES(MatisseCommon::LifecycleComponent)
public:

    explicit ImageProvider(QObject *parent, QString name, QString comment, quint16 outNumber);
    virtual ~ImageProvider();



    ///
    /// \brief Retourne le commentaire associé au composant.
    /// \return
    ///
    QString comment() const;

    ///
    /// \brief Retourne le nombre de port de ce fournisseur.
    /// \return
    ///
    quint16 outNumber() {return _outNumber;}



    //
    // Methodes à surcharger - DEBUT
    //
    ///
    /// \brief Retourne l'ImageSet associé à un port de sortie
    /// \param port
    /// \return
    ///
    virtual ImageSet * imageSet(quint16 outPort) = 0;
    //
    // Methodes à surcharger - FIN
    //


    bool isRealTime() const;
protected:
    void setIsRealTime(bool isRealTime);

signals:
    void signal_userInformation(QString userText);
    void signal_processCompletion(quint8 percentComplete);
    void signal_showInformationMessage(QString title, QString text);
    void signal_showErrorMessage(QString title, QString text);
    void signal_show3DFileOnMainView(QString filepath_p);
    void signal_addRasterFileToMap(QString filepath_p);
    void signal_addPolygonToMap(basicproc::Polygon polygon_p, QString polyInsideColor_p, QString layerName_p);
    void signal_addPolylineToMap(basicproc::Polygon polygon_p, QString polyColor_p, QString layerName_p);
    void signal_addQGisPointsToMap(QList<QgsPoint> pointsList_p, QString pointsColor_p, QString layerName_p);

private:
    QString _comment;
    quint16 _outNumber;
    bool _isRealTime;

};
}
Q_DECLARE_INTERFACE(MatisseCommon::ImageProvider, "Chrisar.ImageProvider/1.1")
#endif // IMAGEPROVIDER_H

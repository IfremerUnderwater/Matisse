
#include <QList>
#include <QtDebug>
#include <QTranslator>

#include "Server.h"
#include "FileImage.h"
#include "ImageSet.h"
#include "Dim2FileReader.h"
#include "PictureFileSet.h"
#include "Context.h"
#include "MatisseParameters.h"
#include "AssemblyGui.h"
#include <qgsapplication.h>

using namespace MatisseServer;
using namespace MatisseTools;

int main(int argc, char *argv[])
{

    // Init QGIS
#ifdef WIN32
    QProcessEnvironment env;
    QString oswgeo4w = env.systemEnvironment().value("OSGEO4W_ROOT");
    QgsApplication::setPrefixPath(oswgeo4w+"\\apps\\qgis", true);
#ifdef QT_DEBUG
    qDebug() << "Load Debug versions of plugins";
    QgsApplication::setPluginPath(oswgeo4w+"\\apps\\qgis\\pluginsd");
#endif
#else
    QgsApplication::setPrefixPath("/usr", true);

#endif

    QgsApplication::initQgis();
    QgsApplication a(argc, argv, true);

    QTranslator matisseTranslator;
    matisseTranslator.load("MatisseServer_en");
    a.installTranslator(&matisseTranslator);

    QTranslator toolsTranslator;
    toolsTranslator.load("MatisseTools_en");
    a.installTranslator(&toolsTranslator);


    qDebug() << QgsApplication::showSettings();
    QString testLaunch("testLaunch");
    if (argc==2 && testLaunch== argv[1])
    {
        qDebug() << testLaunch;
        Server server;
        server.setSettingsFile();
        server.init();

        server.xmlTool().readAssemblyFile("Assemblage_1.xml");
        JobDefinition * jobDef = server.xmlTool().getJob("job2");

        server.processJob(*jobDef);

        // Attente 1 seconde pour les flusher les logs
        QTime dieTime= QTime::currentTime().addSecs(1);
           while( QTime::currentTime() < dieTime ) {
               QCoreApplication::processEvents(QEventLoop::AllEvents, 100);
           }

           delete jobDef;

        return 0;
    }

    QString settingsFile = "";
    if (argc > 1) {
        settingsFile = argv[1];
    }
    AssemblyGui w(settingsFile);

    if (!w.isShowable()) {
        return -1;
    }

    w.show();

    return a.exec();


}

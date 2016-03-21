
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
#include "WelcomeDialog.h"
#include <qgsapplication.h>
#include <QStyleFactory>
using namespace MatisseServer;
using namespace MatisseTools;

void myMessageOutput(QtMsgType type, const char *msg)
{
    switch (type) {
    case QtDebugMsg:
        fprintf(stdout, "INFO: %s\n", msg);
        fflush(stdout);
        break;
    case QtWarningMsg:
        fprintf(stdout, "WARN: %s\n", msg);
        fflush(stdout);
        break;
    case QtCriticalMsg:
        fprintf(stderr, "ERROR: %s\n", msg);
        fflush(stderr);
        break;
    case QtFatalMsg:
        fprintf(stderr, "FATAL: %s\n", msg);
        fflush(stderr);
        abort();
    }
}

int main(int argc, char *argv[])
{
    // Define log handler
    qInstallMsgHandler(myMessageOutput);

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
    qDebug() << "else";
    QgsApplication::setPrefixPath("/usr", true);

#endif


    QgsApplication::initQgis();
    QgsApplication a(argc, argv, true);

    setlocale(LC_ALL, "C");

    //a.setStyle(QStyleFactory::create("Fusion"));

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
    w.setObjectName("_MW_assemblyGui");

    if (!w.isShowable()) {
        return -1;
    }

    // Afficher en premier l'écran d'accueil
    WelcomeDialog wd(&w);
    wd.setObjectName("_D_welcomeDialog");
    wd.setWindowFlags(Qt::Dialog | Qt::FramelessWindowHint);
    wd.show();

    w.loadDefaultStyleSheet();
    w.setWindowFlags(Qt::FramelessWindowHint);//| Qt::WindowMinimizeButtonHint);

    return a.exec();
}

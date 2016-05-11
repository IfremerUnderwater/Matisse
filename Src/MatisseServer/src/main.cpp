
#include <QList>
#include <QtDebug>
#include <QTranslator>
#include <QStyleFactory>
#include <qgsapplication.h>

#include "Server.h"
#include "FileImage.h"
#include "ImageSet.h"
#include "Dim2FileReader.h"
#include "PictureFileSet.h"
#include "Context.h"
#include "MatisseParameters.h"
#include "AssemblyGui.h"
#include "WelcomeDialog.h"
#include "SystemDataManager.h"
#include "ProcessDataManager.h"

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

    // Define default encoding for all text streaming
    QTextCodec::setCodecForLocale(QTextCodec::codecForName("UTF-8"));

    qDebug() << QgsApplication::showSettings();
//    QString testLaunch("testLaunch");
//    if (argc==2 && testLaunch== argv[1])
//    {
//        qDebug() << testLaunch;
//        Server server;
//        server.setSettingsFile();
//        server.init();

//        server.xmlTool().readAssemblyFile("Assemblage_1.xml");
//        JobDefinition * jobDef = server.xmlTool().getJob("job2");

//        server.processJob(*jobDef);

//        // Attente 1 seconde pour les flusher les logs
//        QTime dieTime= QTime::currentTime().addSecs(1);
//        while( QTime::currentTime() < dieTime ) {
//            QCoreApplication::processEvents(QEventLoop::AllEvents, 100);
//        }

//        delete jobDef;

//        return 0;
//    }

    QString settingsFile = "";
    if (argc > 1) {
        settingsFile = argv[1];
    }

    SystemDataManager systemDataManager;
    systemDataManager.readMatisseSettings("config/MatisseSettings.xml");
    QString dataRootDir = systemDataManager.getDataRootDir();
    QString userDataPath = systemDataManager.getUserDataPath();
    ProcessDataManager processDataManager(dataRootDir, userDataPath);

    AssemblyGui w;
    w.setObjectName("_MW_assemblyGui");
    w.setSystemDataManager(&systemDataManager);
    w.setProcessDataManager(&processDataManager);
    w.init();

//    if (!w.isShowable()) {
//        return -1;
//    }

    // Afficher en premier l'écran d'accueil
    WelcomeDialog wd(&w);
    wd.setObjectName("_D_welcomeDialog");
    wd.setWindowFlags(Qt::Dialog | Qt::FramelessWindowHint);
    wd.show();

    w.loadDefaultStyleSheet();
    w.setWindowFlags(Qt::FramelessWindowHint);//| Qt::WindowMinimizeButtonHint);

    return a.exec();
}

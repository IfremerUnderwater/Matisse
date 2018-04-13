
#include <QList>
#include <QtDebug>
#include <QTranslator>
#include <QStyleFactory>
//#include <qgsapplication.h>
//#include <qgsproviderregistry.h>
#include <QApplication>

#include "FileUtils.h"
#include "Server.h"
#include "FileImage.h"
#include "ImageSet.h"
#include "Dim2FileReader.h"
#include "PictureFileSet.h"
#include "Context.h"
#include "MatisseParameters.h"
#include "AssemblyGui.h"
//#include "WelcomeDialog.h"
#include "SystemDataManager.h"
#include "ProcessDataManager.h"

using namespace MatisseServer;
using namespace MatisseTools;

//extern "C" void __stdcall OutputDebugStringA(
//  const char *lpOutputString
//);

//extern "C" int __stdcall  SetEnvironmentVariableA(
//  const char * lpName,
//  const char * lpValue
//);

//extern "C" int __stdcall  GetEnvironmentVariableA(
//   const char * lpName,
//   char *  lpBuffer,
//  int   nSize
//);


void myMessageOutput(QtMsgType type, const QMessageLogContext &, const QString &msg)
{
    QByteArray localMsg = msg.toLocal8Bit();

    //OutputDebugStringA(localMsg.constData());

    switch (type) {
    case QtInfoMsg:
        break;
    case QtDebugMsg:
        fprintf(stdout, "INFO: %s\n", localMsg.constData());
        fflush(stdout);
        break;
    case QtWarningMsg:
        fprintf(stdout, "WARN: %s\n", localMsg.constData());
        fflush(stdout);
        break;
    case QtCriticalMsg:
        fprintf(stderr, "ERROR: %s\n", localMsg.constData());
        fflush(stderr);
        break;
    case QtFatalMsg:
        fprintf(stderr, "FATAL: %s\n", localMsg.constData());
        fflush(stderr);
        QMessageBox mbx;
        mbx.setText(msg);
        mbx.setInformativeText("FATAL....abort()");
        mbx.exec();
        //abort();
        exit(-1);
        break;
    }
}

int main(int argc, char *argv[])
{
//#ifdef WIN32
//    //SetEnvironmentVariableA("QT_PLUGIN_PATH","C:\\Program Files\\qgis2.99.0\\plugins");

//    char plugins[1024];
//    char base[1024];
//    GetEnvironmentVariableA("OSGEO4W_ROOT", base, sizeof(base));
//    strcpy(plugins,base);
//    strcat(base,"apps\\qgis");
//    strcat(plugins,"apps\\qgis\\plugins");
//    SetEnvironmentVariableA("QT_PLUGIN_PATH",plugins);
//     qDebug() << "plugins:" << plugins;
//      qDebug() << "base:" <<base;
//#endif

    setlocale(LC_ALL, "C");

    QApplication a(argc,argv);

    // Define log handler
#if (QT_VERSION >= QT_VERSION_CHECK(5, 0, 0))
     qInstallMessageHandler(myMessageOutput);
#else
    qInstallMsgHandler(myMessageOutput);
#endif

    // Init QGIS
    //QgsApplication a(argc, argv, true);
    //QgsApplication::initQgis();

#ifdef WIN32
    //QProcessEnvironment env;
    //QString oswgeo4w = env.systemEnvironment().value("OSGEO4W_ROOT");
    //QgsApplication::setPluginPath(oswgeo4w+"\\apps\\qgis\\plugins");

//    //QgsApplication::setPrefixPath(oswgeo4w+"\\apps\\qgis", true);
//    //************************TEST
    // Instantiate Provider Registry
    //QgsProviderRegistry::instance("C:\\Program Files\\qgis2.99.0\\plugins");
    //QgsProviderRegistry::instance(oswgeo4w+"\\apps\\qgis\\plugins");

    //QgsApplication::setPrefixPath("C:\\Program Files\\qgis2.99.0",true);
    //QgsApplication::setPrefixPath(oswgeo4w+"\\apps\\qgis",true);

//#ifdef QT_DEBUG
//    qDebug() << "Load Debug versions of plugins";
//    //************************TEST
//    //QgsApplication::setPluginPath(oswgeo4w+"\\apps\\qgis\\pluginsd");
//    //QgsApplication::setPluginPath(oswgeo4w+"\\apps\\qgis\\plugins");
//    QgsApplication::setPluginPath("C:\\Program Files\\qgis2.99.0\\bin\\plugins");
//#endif

#else
//    qDebug() << "else";
//    QgsApplication::setPrefixPath("/usr", true);

#endif


    qRegisterMetaType< basicproc::Polygon >();

    /* Define default encoding for all text streaming */
    QTextCodec::setCodecForLocale(QTextCodec::codecForName("UTF-8"));

//    qDebug() << "**********************************";
//    qDebug() << QgsApplication::showSettings();
//    qDebug() << "**********************************";

    /* Clean all temp directories created during previous sessions */
    FileUtils::removeAllTempDirectories();

    /* Create managers to be injected */
    SystemDataManager systemDataManager;
    systemDataManager.readMatisseSettings("config/MatisseSettings.xml");
    QString dataRootDir = systemDataManager.getDataRootDir();
    QString userDataPath = systemDataManager.getUserDataPath();
    ProcessDataManager processDataManager(dataRootDir, userDataPath);

    /* Create main window */
    AssemblyGui w;
    w.setObjectName("_MW_assemblyGui");
    w.setSystemDataManager(&systemDataManager);
    w.setProcessDataManager(&processDataManager);
    w.init();
    w.loadDefaultStyleSheet();
    w.setWindowFlags(Qt::FramelessWindowHint);

    int ret = a.exec();
//    int ret = a.exec();
//    QgsApplication::exitQgis();
    return ret;
}

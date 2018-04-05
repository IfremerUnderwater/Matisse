
#include <QList>
#include <QtDebug>
#include <QTranslator>
#include <QStyleFactory>
#include <qgsapplication.h>

#include "FileUtils.h"
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

void myMessageOutput(QtMsgType type, const QMessageLogContext &, const QString &msg)
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
#if (QT_VERSION >= QT_VERSION_CHECK(5, 0, 0))
     qInstallMessageHandler(myMessageOutput);
#else
    qInstallMsgHandler(myMessageOutput);
#endif
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

    qRegisterMetaType< basicproc::Polygon >();

    /* Define default encoding for all text streaming */
    QTextCodec::setCodecForLocale(QTextCodec::codecForName("UTF-8"));

    qDebug() << QgsApplication::showSettings();

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

    return a.exec();
}

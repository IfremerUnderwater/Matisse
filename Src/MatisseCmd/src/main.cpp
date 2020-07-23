#include <QStyle>
#include <QDesktopWidget>
#include <QList>
#include <QtDebug>
#include <QTranslator>
#include <QStyleFactory>
#include <QApplication>
#include <QProcessEnvironment>

#include "FileUtils.h"
#include "FileImage.h"
#include "ImageSet.h"
#include "Dim2FileReader.h"
#include "PictureFileSet.h"
#include "Context.h"
#include "MatisseParameters.h"
#include "JobLauncher.h"
//#include "WelcomeDialog.h"
#include "SystemDataManager.h"
#include "ProcessDataManager.h"

using namespace MatisseTools;
using namespace MatisseCmd;

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
        //abort();
        exit(-1);
        break;
    }
}

int main(int argc, char *argv[])
{
    // set all locales to avoid numbers with , instead of .
    setlocale(LC_ALL, "C");
    QLocale::setDefault(QLocale::C);
    //std::setlocale(LC_ALL, "C");

    QApplication a(argc,argv);

    // Define log handler
#if (QT_VERSION >= QT_VERSION_CHECK(5, 0, 0))
    qInstallMessageHandler(myMessageOutput);
#else
    qInstallMsgHandler(myMessageOutput);
#endif

    qRegisterMetaType< basicproc::Polygon >();

    /* Define default encoding for all text streaming */
    QTextCodec::setCodecForLocale(QTextCodec::codecForName("UTF-8"));

    /* Clean all temp directories created during previous sessions */
    FileUtils::removeAllTempDirectories();

    //QProcessEnvironment env = QProcessEnvironment::systemEnvironment();
    //if (!env.contains("MATISSE_PATH")) 
    //{
    //  qFatal("Environment variable 'MATISSE_PATH' not set");
    //}

    //QString matisse_bin_path = env.value("MATISSE_PATH");
    QString matisse_bin_path = ".";
    qDebug() << "Bin path : " << QDir(matisse_bin_path).absolutePath();

    /* Checking arguments */
    if (argc < 3) 
    {
      qFatal("Usage: MatisseCmd <job root path> <job name>");
    }

    char *arg_root = argv[1];
    QString job_root_path(arg_root);
    QDir job_root_dir(job_root_path);
    if (!job_root_dir.exists()) 
    {
      qFatal(QString("Job directory '%1 not found").arg(job_root_path).toLatin1());
    }

    char *arg_job = argv[2];
    QString job_name(arg_job);
    QString job_xml_path = job_root_path + QDir::separator() + "xml";
    QString job_file_path = job_xml_path + QDir::separator() + "jobs" +
                            QDir::separator() + job_name + ".xml";
    QFile job_file(job_file_path);
    if (!job_file.exists()) {
      qFatal(
          QString("Job file '%1 not found").arg(job_file_path).toLatin1());
    }

    /* Create managers to be injected */
    QString settings_path = matisse_bin_path + QDir::separator() + "config" 
      + QDir::separator() + "MatisseSettings.xml";

    //SystemDataManager systemDataManager(matisse_bin_path);
    SystemDataManager systemDataManager;
    //systemDataManager.readMatisseSettings("config/MatisseSettings.xml");
    systemDataManager.readMatisseSettings(settings_path);
    //QString dataRootDir = systemDataManager.getDataRootDir();
    //QString userDataPath = systemDataManager.getUserDataPath();
    //ProcessDataManager processDataManager(dataRootDir, userDataPath);
    ProcessDataManager processDataManager(job_root_path, job_xml_path);

    /* Create main class and set params */
    JobLauncher jl;

    jl.setSystemDataManager(&systemDataManager);
    jl.setProcessDataManager(&processDataManager);
    jl.init();

    //jl.launchJob("bibi");
    jl.launchJob(job_name);

    int ret = a.exec();
    return ret;
}

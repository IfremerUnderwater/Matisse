#include <QStyle>
#include <QDesktopWidget>
#include <QList>
#include <QtDebug>
#include <QTranslator>
#include <QStyleFactory>
#include <QApplication>
#include <QProcessEnvironment>

#include "file_utils.h"
#include "file_image.h"
#include "image_set.h"
#include "dim2_file_reader.h"
#include "picture_file_set.h"
#include "Context.h"
#include "matisse_parameters.h"
#include "job_launcher.h"
#include "system_data_manager.h"
#include "process_data_manager.h"

using namespace matisse;

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

    qRegisterMetaType< basic_processing::Polygon >();

    /* Define default encoding for all text streaming */
    QTextCodec::setCodecForLocale(QTextCodec::codecForName("UTF-8"));

    /* Clean all temp directories created during previous sessions */
    FileUtils::removeAllTempDirectories();

    QString matisse_bin_path = ".";
    qDebug() << "MatisseCmd bin path : " << QDir(matisse_bin_path).absolutePath();

    /* Checking arguments */
    if (argc < 3) 
    {
      qFatal("Usage: MatisseCmd <data root path> <job name>");
    }

    char *arg_root = argv[1];
    QString data_root_path(arg_root);
    qDebug() << QString("MatisseCmd arg 1 (data root path) : %1")
                    .arg(data_root_path);

    QDir data_root_dir(data_root_path);
    if (!data_root_dir.exists()) 
    {
      qFatal(QString("Data root directory '%1' not found").arg(data_root_path).toLatin1());
    }
     
    char *arg_job = argv[2];
    QString job_name(arg_job);
    qDebug() << QString("MatisseCmd arg 2 (job name) : %1")
                    .arg(job_name);

    QString data_xml_path = data_root_path + QDir::separator() + "xml";
    QString job_file_path = data_xml_path + QDir::separator() + "jobs" +
                            QDir::separator() + job_name + ".xml";
    QFile job_file(job_file_path);
    if (!job_file.exists()) {
      qFatal(
          QString("Job file '%1 not found").arg(job_file_path).toLatin1());
    }

    /* Create managers to be injected */
    QString settings_path = matisse_bin_path + QDir::separator() + "config" 
      + QDir::separator() + "MatisseSettings.xml";

    SystemDataManager* system_data_manager = SystemDataManager::instance();
    system_data_manager->init();
    system_data_manager->readMatisseSettings(settings_path);
    ProcessDataManager* process_data_manager = ProcessDataManager::instance();
    process_data_manager->init(data_root_path, data_xml_path);

    /* Create main class and set params */
    JobLauncher jl;

    jl.init();

    jl.launchJob(job_name);

    int ret = a.exec();
    return ret;
}

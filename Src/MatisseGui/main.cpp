﻿#include <QStyle>
#include <QDesktopWidget>
#include <QList>
#include <QtDebug>
#include <QTranslator>
#include <QStyleFactory>
#include <QApplication>

#include "file_utils.h"
#include "file_image.h"
#include "image_set.h"
#include "dim2_file_reader.h"
#include "picture_file_set.h"
#include "Context.h"
#include "matisse_parameters.h"
#include "main_gui.h"
#include "system_data_manager.h"
#include "process_data_manager.h"
#include "camera_manager.h"
#include "MatisseConfig.h"
#include <QSettings>
#include "network_client.h"
#include "secure_connection_wrapper.h"
#include "sftp_client.h"
#include "ssh_client.h"
#include "assembly_helper.h"
#include "job_helper.h"
#include "remote_job_helper.h"

using namespace matisse;

void myMessageOutput(QtMsgType _type, const QMessageLogContext &, const QString &_msg)
{
    QByteArray local_msg = _msg.toLocal8Bit();

    switch (_type) {
    case QtInfoMsg:
        break;
    case QtDebugMsg:
        fprintf(stdout, "INFO: %s\n", local_msg.constData());
        fflush(stdout);
        break;
    case QtWarningMsg:
        fprintf(stdout, "WARN: %s\n", local_msg.constData());
        fflush(stdout);
        break;
    case QtCriticalMsg:
        fprintf(stderr, "ERROR: %s\n", local_msg.constData());
        fflush(stderr);
        break;
    case QtFatalMsg:
        fprintf(stderr, "FATAL: %s\n", local_msg.constData());
        fflush(stderr);
        QMessageBox mbx;
        mbx.setText(_msg);
        mbx.setInformativeText("FATAL....abort()");
        mbx.exec();
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
#ifndef _MSC_VER
    std::setlocale(LC_ALL, "C");
#endif // !1

    QLoggingCategory::setFilterRules("qtc.ssh.debug=false");

    QApplication a(argc,argv);

    // Define customized log handler
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


    /* Check if we need to deploy and eventually make it */
    QSettings matisse_settings("Ifremer","Matisse");
    QString current_version = QString("%1.%2.%3").arg(MATISSE_VERSION_MAJOR).arg(MATISSE_VERSION_MINOR).arg(MATISSE_VERSION_PATCH);
    bool deployment_needed = false;

    if (matisse_settings.contains("Matisse/Version"))
    {
        QString deployed_version = matisse_settings.value("Matisse/Version").toString();
        if (current_version != deployed_version)
            deployment_needed = true;
    }
    else
    {
        deployment_needed = true;
    }

    matisse_settings.setValue("Matisse/Version",QString("%1.%2.%3").arg(MATISSE_VERSION_MAJOR).arg(MATISSE_VERSION_MINOR).arg(MATISSE_VERSION_PATCH) );

    //if (deployment_needed) // Deploy all the time for the moment
    //{
        CameraManager::instance().deployDefaultCamerasToAppData();
    //}

    /* Check working directory path */
    QString matisse_bin_path = ".";
    QString config_file_path = "config/MatisseSettings.xml";

    qDebug() << "MatisseCmd bin path : " << QDir(matisse_bin_path).absolutePath();
    qDebug() << "MatisseCmd config file path : " << QFileInfo(config_file_path).absoluteFilePath();

    /* Create managers to be injected */
    CameraManager::instance().initializeFromDataBase();
    SystemDataManager* system_data_manager = SystemDataManager::instance();
    system_data_manager->init();
    system_data_manager->readMatisseSettings(config_file_path);
    QString data_root_dir = system_data_manager->getDataRootDir();
    QString user_data_path = system_data_manager->getUserDataPath();
    ProcessDataManager* process_data_manager = ProcessDataManager::instance();
    process_data_manager->init(data_root_dir, user_data_path);

    /* Create UI helpers */
    AssemblyHelper assembly_helper;
    JobHelper job_helper;
    ImportExportHelper import_export_helper;

    /* Create remote process gateways and UI helper */
    ConnectionWrapper* secure_network_command_handler = new SecureConnectionWrapper();
    NetworkClient* ssh_client = new SshClient();
    ssh_client->setConnectionWrapper(secure_network_command_handler);
    ConnectionWrapper* secure_network_file_handler = new SecureConnectionWrapper();
    NetworkClient* sftp_client = new SftpClient();
    sftp_client->setConnectionWrapper(secure_network_file_handler);
    RemoteJobHelper remote_job_helper;
    remote_job_helper.setSshClient(ssh_client);
    remote_job_helper.setSftpClient(sftp_client);

    /* Create main window and set params */
    MainGui w;

    w.setGeometry(
                QStyle::alignedRect(
                    Qt::LeftToRight,
                    Qt::AlignCenter,
                    w.size(),
                    qApp->desktop()->availableGeometry()
                    )
                );

    w.setObjectName("_MW_assemblyGui");
    w.setAssemblyHelper(&assembly_helper);
    w.setJobHelper(&job_helper);
    w.setImportExportHelper(&import_export_helper);
    w.setRemoteJobHelper(&remote_job_helper);
    w.init();
    //w.loadDefaultStyleSheet();
    w.sl_showApplicationMode(POST_PROCESSING); // open directly to the most used mode ie : post processing

    int ret = a.exec();
    return ret;
}

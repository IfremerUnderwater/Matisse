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
#include "network_commons.h"
#include "network_client.h"
#include "network_client_file_transfer.h"
#include "network_client_shell.h"
#include "network_connector_qssh.h"
// #include "network_connector_qftp.h"
#include "network_connector_ftpclient.h"
#include "assembly_helper.h"
#include "job_helper.h"
#include "remote_job_helper.h"

using namespace matisse;
using namespace network_tools;

#ifdef _WIN32
// Use discrete GPU by default.
extern "C" {
    // http://developer.download.nvidia.com/devzone/devcenter/gamegraphics/files/OptimusRenderingPolicies.pdf
    __declspec(dllexport) DWORD NvOptimusEnablement = 0x00000001;

    // http://developer.amd.com/community/blog/2015/10/02/amd-enduro-system-for-developers/
    __declspec(dllexport) int AmdPowerXpressRequestHighPerformance = 1;
}
#endif

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

    QCoreApplication::setAttribute(Qt::AA_ShareOpenGLContexts);

    QLoggingCategory::setFilterRules("qtc.ssh.debug=false");

    QApplication a(argc,argv);

    // Define customized log handler
#if (QT_VERSION >= QT_VERSION_CHECK(5, 0, 0))
    qInstallMessageHandler(myMessageOutput);
#else
    qInstallMsgHandler(myMessageOutput);
#endif

    qRegisterMetaType<basic_processing::Polygon >();
    qRegisterMetaType<eConnectionError>();
    qRegisterMetaType<eTransferError>("eTransferError");
    qRegisterMetaType<NetworkAction::eNetworkActionType>("NetworkAction::eNetworkActionType");
    qRegisterMetaType < QList<network_tools::NetworkFileInfo*> >();

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
    NetworkConnector* ssh_handler = new NetworkConnectorQSsh();
    NetworkClient* ssh_client = new NetworkClientShell();
    ssh_client->setConnector(ssh_handler);

    //NetworkConnector* ftp_handler = new NetworkConnectorQFtp();
    NetworkConnector* ftp_handler = new NetworkConnectorFTPClient();
    NetworkClient* ftp_client = new NetworkClientFileTransfer();
    ftp_client->setConnector(ftp_handler);

    NetworkConnector* sftp_handler = new NetworkConnectorQSsh();
    NetworkClient* sftp_client = new NetworkClientFileTransfer();
    sftp_client->setConnector(sftp_handler);

    RemoteJobHelper remote_job_helper;
    remote_job_helper.registerNetworkFileClient(eFileTransferProtocol::FTP, ftp_client);
    remote_job_helper.registerNetworkFileClient(eFileTransferProtocol::SFTP, sftp_client);
    remote_job_helper.registerNetworkShellClient(eShellProtocol::SSH, ssh_client);

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

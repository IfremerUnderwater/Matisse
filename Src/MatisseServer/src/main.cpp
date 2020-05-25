//#include "WinSocket.h"

#include <QStyle>
#include <QDesktopWidget>
#include <QList>
#include <QtDebug>
#include <QTranslator>
#include <QStyleFactory>
#include <QApplication>

#include "FileUtils.h"
#include "FileImage.h"
#include "ImageSet.h"
#include "Dim2FileReader.h"
#include "PictureFileSet.h"
#include "Context.h"
#include "MatisseParameters.h"
#include "AssemblyGui.h"
#include "SystemDataManager.h"
#include "ProcessDataManager.h"
#include "AbstractSshClient.h"
#include "SshClientStub.h"
#include "LibSshClient.h"
#include "RemoteJobManager.h"

using namespace MatisseServer;
using namespace MatisseTools;




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

    // set all locales to avoid numbers with , instead of .
    setlocale(LC_ALL, "C");
    QLocale::setDefault(QLocale::C);
#ifndef _MSC_VER
    std::setlocale(LC_ALL, "C");
#endif // !1

    QLoggingCategory::setFilterRules("qtc.ssh.debug=false");

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

    /* To retrieve from preferences */
    QString host = "51.210.7.224";
    QString username = "matisse";
    QString password = "toto";
    SshClientCredentials *creds = new SshClientCredentials(username, password);

//    AbstractSshClient *sshClient = new SshClientStub(host, creds);
    AbstractSshClient *sshClient = new LibSshClient(host, creds);
    RemoteJobManager remoteJobManager;
    remoteJobManager.setSshClient(sshClient);


    /* Create main window and set params */
    AssemblyGui w;

    w.setGeometry(
                QStyle::alignedRect(
                    Qt::LeftToRight,
                    Qt::AlignCenter,
                    w.size(),
                    qApp->desktop()->availableGeometry()
                    )
                );

    w.setObjectName("_MW_assemblyGui");
    w.setSystemDataManager(&systemDataManager);
    w.setProcessDataManager(&processDataManager);
    w.setRemoteJobManager(&remoteJobManager);
    w.init();
    w.loadDefaultStyleSheet();
    w.setWindowFlags(Qt::FramelessWindowHint);

    int ret = a.exec();
    return ret;
}

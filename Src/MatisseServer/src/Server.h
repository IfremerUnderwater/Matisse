#ifndef SERVER_H
#define SERVER_H

#include <QString>
#include <QTcpSocket>
#include <QThread>
#include <QTcpServer>
#include <QNetworkInterface>
#include <QModelIndex>
#include <QCryptographicHash>

#include "Processor.h"
#include "ImageProvider.h"
#include "RasterProvider.h"
#include "JobServer.h"
#include "FileUtils.h"
#include "MatisseParametersManager.h"
#include "SystemDataManager.h"
#include "ProcessDataManager.h"

using namespace MatisseCommon;
using namespace MatisseTools;

namespace MatisseServer {

enum ApplicationMode {
    PROGRAMMING,
    REAL_TIME,
    DEFERRED_TIME,
    APP_CONFIG
};

class AssemblyGui;
class Server;
class JobTask : public QObject{
    Q_OBJECT

public:
    explicit JobTask(ImageProvider *imageProvider, QList<Processor *> processors, RasterProvider *rasterProvider , JobDefinition &jobDefinition, MatisseParameters *parameters);
    virtual ~JobTask();
    void stop(bool cancel=false);
    JobDefinition &jobDefinition() const;

    QStringList resultFileNames() const;

    bool isCancelled() const;

    void setMainGui(AssemblyGui *mainGui);

signals:
    void signal_jobShowImageOnMainView(QString jobName, Image *image);
    void signal_userInformation(QString userText);
    void signal_processCompletion(quint8 percentComplete);
    void signal_jobStopped();

public slots:
    void slot_start();
    void slot_stop();
    void slot_showImageOnMainView(Image *image);
    void slot_userInformation(QString userText);
    void slot_processCompletion(quint8 percentComplete);

private:
    AssemblyGui* _mainGui;
    Context* _context;
    ImageProvider* _imageProvider;
    QList<Processor*> _processors;
    RasterProvider* _rasterProvider;
    JobDefinition &_jobDefinition;
    MatisseParameters *_matParameters;
    MatisseParameters *_assemblyParameters;
    QStringList _resultFileNames;
    volatile bool _isCancelled;



};

class Server : public QObject
{
    Q_OBJECT
    
public:
    explicit Server(QObject *parent = 0);
    virtual ~Server();

    //bool setSettingsFile(QString settings = "");
    void init();
    void setMainGui(AssemblyGui* mainGui_p);

    QList<Processor*> const getAvailableProcessors();
    QList<ImageProvider*> const getAvailableImageProviders();
    QList<RasterProvider*> const getAvailableRasterProviders();

    void addParametersForImageProvider(QString name);
    void addParametersForProcessor(QString name);
    void addParametersForRasterProvider(QString name);

    bool removeModuleAndExpectedParameters(QString name);

    bool processJob(JobDefinition&  jobDefinition);
    bool isProcessingJob();
    bool stopJob(bool cancel=false);
    bool errorFlag();
    QString messageStr();
//    Xml& xmlTool();
    MatisseParametersManager * parametersManager() { return _dicoParamMgr; }

    void setSystemDataManager(SystemDataManager *systemDataManager);
    void setProcessDataManager(ProcessDataManager *processDataManager);

signals:
    void signal_jobShowImageOnMainView(QString jobName, Image *image);
    void signal_userInformation(QString userText);
    void signal_processCompletion(quint8 percentComplete);
    void signal_jobProcessed(QString jobName, bool isCancelled);

private slots:
    void slot_currentJobProcessed();

private:
    MatisseParameters* buildMatisseParameters(JobDefinition &job);
    bool buildJobTask( AssemblyDefinition &assembly, JobDefinition &jobDefinition, MatisseParameters *matisseParameters);
    void setMessageStr(QString messageStr = "", bool error = true);
    bool loadParametersDictionnary();

    AssemblyGui* _mainGui;
    SystemDataManager *_systemDataManager;
    ProcessDataManager *_processDataManager;
    JobServer *_jobServer;
    JobTask* _currentJob;
    QThread* _thread;
    //Xml _xmlTool;
    MatisseParametersManager* _dicoParamMgr;
    QHash<QString, Processor*> _processors;
    QHash<QString, ImageProvider*> _imageProviders;
    QHash<QString, RasterProvider*> _rasterProviders;
    QHash<QString, QList<MatisseParameter> > _expectedParametersByModule;
    QString _messageStr;
    bool _errorFlag;

};
}

#endif // SERVER_H

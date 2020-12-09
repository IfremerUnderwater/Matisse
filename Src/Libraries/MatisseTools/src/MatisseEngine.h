#ifndef MATISSEENGINE_H
#define MATISSEENGINE_H

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

namespace MatisseTools {

enum ApplicationMode {
    PROGRAMMING,
    REAL_TIME,
    POST_PROCESSING,
    APP_CONFIG
};

class MatisseEngine;
class JobTask : public QObject{
    Q_OBJECT

public:
    explicit JobTask(ImageProvider *imageProvider, QList<Processor *> processors, RasterProvider *rasterProvider , JobDefinition &jobDefinition, MatisseParameters *parameters);
    virtual ~JobTask();
    void stop(bool cancel=false);
    JobDefinition &jobDefinition() const;

    QStringList resultFileNames() const;

    bool isCancelled() const;

    void setJobLauncher(QObject *jobLauncher);
    void setIsServerMode(bool _is_server_mode);

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
    void slot_fatalError();

private:
    QObject* _jobLauncher;
    Context* _context;
    ImageProvider* _imageProvider;
    QList<Processor*> _processors;
    RasterProvider* _rasterProvider;
    JobDefinition &_jobDefinition;
    MatisseParameters *_matParameters;
    MatisseParameters *_assemblyParameters;
    QStringList _resultFileNames;
    volatile bool _isCancelled;
    bool m_is_server_mode = false;
};

class MatisseEngine : public QObject
{
    Q_OBJECT

public:
    explicit MatisseEngine(QObject *parent = 0, bool _is_server_mode=false);
    virtual ~MatisseEngine();

    //bool setSettingsFile(QString settings = "");
    void init();
    void setJobLauncher(QObject* jobLauncher);

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

    bool m_is_server_mode;
    QObject* _jobLauncher;
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


#endif // MATISSEENGINE_H

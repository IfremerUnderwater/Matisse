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
#include "Xml.h"

using namespace MatisseCommon;
using namespace MatisseTools;

namespace MatisseServer {

class Server;
class JobTask : public QObject{
    Q_OBJECT

public:
    explicit JobTask(ImageProvider *imageProvider, QList<Processor *> processors, RasterProvider *rasterProvider , JobDefinition &jobDefinition, MatisseParameters *parameters);
    virtual ~JobTask();
    void stop(bool cancel=false);
    JobDefinition &jobDefinition() const;

    QStringList resultFileNames() const;

    volatile bool isCancelled() const;


signals:
    void signal_jobIntermediateResult(QString jobName, Image *image);
    void signal_jobStopped();

public slots:
    void slot_start();
    void slot_stop();


    void slot_intermediateResult(Image *image);

private:
    Context* _context;
    ImageProvider* _imageProvider;
    QList<Processor*> _processors;
    RasterProvider* _rasterProvider;
    JobDefinition &_jobDefinition;
    MatisseParameters *_matParameters;
    QStringList _resultFileNames;
    volatile bool _isCancelled;



};

class Server : public QObject
{
    Q_OBJECT
    
public:
    explicit Server(QObject *parent = 0);
    virtual ~Server();

    bool setSettingsFile(QString settings = "");
    void init();

    QList<Processor*> const getAvailableProcessors();
    QList<ImageProvider*> const getAvailableImageProviders();
    QList<RasterProvider*> const getAvailableRasterProviders();

    bool processJob(JobDefinition&  jobDefinition);
    bool isProcessingJob();
    bool stopJob(bool cancel=false);
    bool errorFlag();
    QString messageStr();
    Xml& xmlTool();

signals:
    void signal_jobIntermediateResult(QString jobName, Image *image);
    void signal_jobProcessed(QString jobName, bool isCancelled);

private slots:
    void slot_currentJobProcessed();

private:
    MatisseParameters* buildMatisseParameters(JobDefinition &job);
    bool buildJobTask( AssemblyDefinition &assembly, JobDefinition &jobDefinition, MatisseParameters *matisseParameters);
    void setMessageStr(QString messageStr = "", bool error = true);

private:
    JobServer *_jobServer;
    JobTask* _currentJob;
    QThread* _thread;
    Xml _xmlTool;
    QHash<QString, Processor*> _processors;
    QHash<QString, ImageProvider*> _imageProviders;
    QHash<QString, RasterProvider*> _rasterProviders;
    QString _messageStr;
    bool _errorFlag;

};
}

#endif // SERVER_H

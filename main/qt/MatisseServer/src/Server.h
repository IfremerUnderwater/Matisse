#ifndef SERVER_H
#define SERVER_H

#include <QString>
#include <QTcpSocket>
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

    bool processJob(JobDefinition&  job);
    bool errorFlag();
    QString messageStr();
   // bool sendDatas(QString data);
    //QString getUserParameters(QString processor, QString user);

    Xml& xmlTool();

signals:
    void signal_jobProcessed(QString jobName);

private:
    bool checkAssembly(MatisseParameters* mosaicParameters, AssemblyDefinition&  assembly);
    MatisseParameters* buildMosaicParameters(JobDefinition &job);
    void setMessageStr(QString messageStr = "", bool error = true);

private:
    JobServer *_jobServer;
    Xml _xmlTool;
    QHash<QString, Processor*> _processors;
    QHash<QString, ImageProvider*> _imageProviders;
    QHash<QString, RasterProvider*> _rasterProviders;
    QString _messageStr;
    bool _errorFlag;

};
}

#endif // SERVER_H

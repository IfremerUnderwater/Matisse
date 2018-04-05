#include <QDebug>
#include "JobDefinition.h"
#include "JobServer.h"

using namespace MatisseTools;

JobServer::JobServer(int port, ProcessDataManager *processDataManager) :
    QObject(NULL),
    _processDataManager(processDataManager),
    _socket(NULL)
{
    connect(&_server, SIGNAL(newConnection()), this, SLOT(slot_clientConnected()));

     _server.listen(QHostAddress(QHostAddress::LocalHost), port);
     qDebug() << "Server Listening on port " << port;
}

void JobServer::sendExecutionNotification(QString name)
{
    QString configCmd = "$JOBEXECUTION:"+name;
    sendCmd(configCmd);
}

void JobServer::slot_clientConnected()
{
    qDebug() << "Client connected";
    if (_socket) {
        if (_socket->isOpen()) {
            _socket->flush();
            _socket->close();
        }
        _socket->disconnect();
        _socket->deleteLater();
    }

    _socket = _server.nextPendingConnection();
    if (!_socket) {
        qWarning() << "No pending connection";
        return;
    }
    connect(_socket, SIGNAL(disconnected()), this, SLOT(slot_clientDisconnected()));
    connect(_socket, SIGNAL(readyRead()), this, SLOT(slot_readData()));

    // Pour un usage futur (auparavant, le chemin des data etait passe par la).
    QString configCmd = "$CONFIG:";
    sendCmd(configCmd);

}

void JobServer::slot_clientDisconnected()
{
    qDebug() << "Client disconnected";
}

void JobServer::slot_readData()
{
    QByteArray datas;
    while (_socket->bytesAvailable() > 0) {
        datas.append(_socket->readAll());
    }
    QString datasStr(datas);
    qDebug() << "Receive: " << datasStr;
    if (datasStr.startsWith("$LISTJOBS")) {
         QString jobCmd;


         foreach (QString jobName, _processDataManager->getJobsNames()) {

             JobDefinition *def= _processDataManager->getJob(jobName);

             bool isExecuted = def->executionDefinition()->executed();
             if(isExecuted) {
                 QDateTime time = def->executionDefinition()->executionDate();
                 QString comment = def->comment();
                 QStringList results = def->executionDefinition()->resultFileNames();
                 QString resultCount = QString::number(results.size());
                 QString assemblyName = def->assemblyName();
                 // Avoid special chars used as delimiters
                 comment.replace(QString(";"), QString(","));
                 comment.replace(QString("^"), QString(" "));
                 jobCmd= "$JOB:"
                         + def->name() +";"
                         + assemblyName +";"
                         + time.toString("dd/MM/yyyy HH:mm") + ";"
                         + comment + ";"
                         + resultCount ;
                 foreach (QString result, results) {
                     jobCmd.append(";");
                     jobCmd.append(result);
                 }
                 sendCmd(jobCmd);
            }
         }
    }

}

bool JobServer::sendCmd(QString data) {
    bool ret = false;
    if (_socket && _socket->isOpen()) {
        data.append("^");
#if (QT_VERSION >= QT_VERSION_CHECK(5, 0, 0))
        if (_socket->write(data.toLatin1()) == data.size()){
            qDebug() << "Send:" << data.toLatin1();
#else
        if (_socket->write(data.toAscii()) == data.size()){
            qDebug() << "Send:" << data.toAscii();
#endif
            ret = _socket->flush();
        }
    }
    return ret;
}

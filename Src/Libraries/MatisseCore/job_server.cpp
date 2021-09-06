#include <QDebug>
#include "job_definition.h"
#include "job_server.h"

namespace matisse {

JobServer::JobServer(int _port) :
    QObject(NULL),
    m_socket(NULL)
{
    connect(&m_server, SIGNAL(newConnection()), this, SLOT(sl_clientConnected()));

     m_server.listen(QHostAddress(QHostAddress::LocalHost), _port);
     qDebug() << "Server Listening on port " << _port;
}

void JobServer::sendExecutionNotification(QString _name)
{
    QString config_cmd = "$JOBEXECUTION:"+_name;
    sendCmd(config_cmd);
}

void JobServer::sl_clientConnected()
{
    qDebug() << "Client connected";
    if (m_socket) {
        if (m_socket->isOpen()) {
            m_socket->flush();
            m_socket->close();
        }
        m_socket->disconnect();
        m_socket->deleteLater();
    }

    m_socket = m_server.nextPendingConnection();
    if (!m_socket) {
        qWarning() << "No pending connection";
        return;
    }
    connect(m_socket, SIGNAL(disconnected()), this, SLOT(sl_clientDisconnected()));
    connect(m_socket, SIGNAL(readyRead()), this, SLOT(sl_readData()));

    // Pour un usage futur (auparavant, le chemin des data etait passe par la).
    QString config_cmd = "$CONFIG:";
    sendCmd(config_cmd);

}

void JobServer::sl_clientDisconnected()
{
    qDebug() << "Client disconnected";
}

void JobServer::sl_readData()
{
    QByteArray data;
    while (m_socket->bytesAvailable() > 0) {
        data.append(m_socket->readAll());
    }
    QString datas_str(data);
    qDebug() << "Receive: " << datas_str;
    if (datas_str.startsWith("$LISTJOBS")) {
         QString job_cmd;

         ProcessDataManager* process_data_manager = ProcessDataManager::instance();

         foreach (QString job_name, process_data_manager->getJobsNames()) {

             JobDefinition *def= process_data_manager->getJob(job_name);

             bool is_executed = def->executionDefinition()->executed();
             if(is_executed) {
                 QDateTime time = def->executionDefinition()->executionDate();
                 QString comment = def->comment();
                 QStringList results = def->executionDefinition()->resultFileNames();
                 QString result_count = QString::number(results.size());
                 QString assembly_name = def->assemblyName();
                 // Avoid special chars used as delimiters
                 comment.replace(QString(";"), QString(","));
                 comment.replace(QString("^"), QString(" "));
                 job_cmd= "$JOB:"
                         + def->name() +";"
                         + assembly_name +";"
                         + time.toString("dd/MM/yyyy HH:mm") + ";"
                         + comment + ";"
                         + result_count ;
                 foreach (QString result, results) {
                     job_cmd.append(";");
                     job_cmd.append(result);
                 }
                 sendCmd(job_cmd);
            }
         }
    }

}

bool JobServer::sendCmd(QString _data) {
    bool ret = false;
    if (m_socket && m_socket->isOpen()) {
        _data.append("^");
#if (QT_VERSION >= QT_VERSION_CHECK(5, 0, 0))
        if (m_socket->write(_data.toLatin1()) == _data.size()){
            qDebug() << "Send:" << _data.toLatin1();
#else
        if (m_socket->write(_data.toAscii()) == _data.size()){
            qDebug() << "Send:" << _data.toAscii();
#endif
            ret = m_socket->flush();
        }
    }
    return ret;
}

} // namespace matisse

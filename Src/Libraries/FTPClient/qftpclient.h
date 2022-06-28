#ifndef QFTPCLIENT_H
#define QFTPCLIENT_H

#include <QObject>
#include <QThread>
#include "threadable_ftpclient.h"

class QFTPClient : public QObject
{
    Q_OBJECT
public:
    explicit QFTPClient(QObject *parent = nullptr);
    void connectToHost(const QString& _host, const QString& _username, const QString& _password, const unsigned& _port);
    void listDir(QString _dir);

private:
    QThread m_ftp_thread;
    ThreadableFTPClient m_ftp_client;

private slots:
    void sl_connected();
    void sl_connectionFailed(QString _err);
    void sl_dirContents(QList<network_tools::NetworkFileInfo*> _dir_contents);


signals:
    void si_connected();
    void si_connectionFailed(QString _err);
    void si_connectToHost(QString _host, QString _username, QString _password, unsigned _port);
    void si_listDir(QString _dir);
    void si_dirContents(QList<network_tools::NetworkFileInfo*> _dir_contents);

};

#endif // QFTPCLIENT_H

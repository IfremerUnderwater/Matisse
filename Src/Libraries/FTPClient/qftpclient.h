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
    void uploadFile(QString _local_file_path, QString _remote_file_path);
    void downloadFile(QString _remote_file_path, QString _local_file_path);
    void uploadDir(QString _local_dir_path, QString _remote_dir_path, bool _recursive);
    void downloadDir(QString _remote_dir_path, QString _local_dir_path, bool _recursive);
    void stopThread();

private:
    QThread m_ftp_thread;
    ThreadableFTPClient m_ftp_client;

private slots:
    void sl_connected();
    void sl_connectionFailed(QString _err);
    void sl_errorOccured(network_tools::eTransferError _error_type, QString _error_msg);
    void sl_dirContents(QList<network_tools::NetworkFileInfo*> _dir_contents);
    void sl_progressUpdate(int _progress);
    void sl_transferFinished();


signals:
    void si_connected();
    void si_connectionFailed(QString _err);
    void si_errorOccured(network_tools::eTransferError _error_type, QString _error_msg);
    void si_connectToHost(QString _host, QString _username, QString _password, unsigned _port);
    void si_listDir(QString _dir);
    void si_dirContents(QList<network_tools::NetworkFileInfo*> _dir_contents);
    void si_progressUpdate(int _progress);
    void si_transferFinished();

    void si_uploadFile(QString _local_file_path, QString _remote_file_path);
    void si_downloadFile(QString _remote_file_path, QString _local_file_path);
    void si_uploadDir(QString _local_dir_path, QString _remote_dir_path, bool _recursive);
    void si_downloadDir(QString _remote_dir_path, QString _local_dir_path, bool _recursive);

};

#endif // QFTPCLIENT_H

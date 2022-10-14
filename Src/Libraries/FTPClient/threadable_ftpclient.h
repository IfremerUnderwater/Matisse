#ifndef THREADABLEFTPCLIENT_H
#define THREADABLEFTPCLIENT_H

#include <QObject>
#include <QString>
#include "FTPClient.h"
#include "network_commons.h"

class ThreadableFTPClient : public QObject
{
    Q_OBJECT

    enum state {
        UPLOADING,
        DOWNLOADING,
        IDLE
    };

public:
    explicit ThreadableFTPClient(QObject *_parent = nullptr);
    static int DLProgressCallback(void* ptr, double dTotalToDownload, double dNowDownloaded, double dTotalToUpload, double dNowUploaded);
    void emitProgress(int _progress);

public slots:
    void sl_connectToHost(QString _host, QString _username, QString _password, unsigned _port=21);
    void sl_listDir(QString _dir);

    bool sl_uploadFile(QString _local_file_path, QString _remote_file_path);
    void sl_downloadFile(QString _remote_file_path, QString _local_file_path);
    void sl_uploadDir(QString _local_dir_path, QString _remote_dir_path, bool _recursive);
    void sl_downloadDir(QString _remote_dir_path, QString _local_dir_path, bool _recursive);

private:
    QString replaceMonthMMMByNumber(const QString& _date_string);

    embeddedmz::CFTPClient m_ftp;
    bool m_connected;
    state m_current_state;

signals:
    void si_connected();
    void si_connectionFailed(QString _err);
    void si_errorOccured(int _error_type, QString _error_msg);
    void si_dirContents(QList<network_tools::NetworkFileInfo*> _contents);
    void si_progressUpdate(int _progress);
    void si_transferFinished();

};

#endif // THREADABLEFTPCLIENT_H

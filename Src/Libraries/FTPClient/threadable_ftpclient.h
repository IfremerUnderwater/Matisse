#ifndef THREADABLEFTPCLIENT_H
#define THREADABLEFTPCLIENT_H

#include <QObject>
#include <QString>
#include "FTPClient.h"
#include "network_commons.h"

class ThreadableFTPClient : public QObject
{
    Q_OBJECT
public:
    explicit ThreadableFTPClient(QObject *_parent = nullptr);

public slots:
    void sl_connectToHost(QString _host, QString _username, QString _password, unsigned _port=21);
    void sl_listDir(QString _dir);

private:
    QString replaceMonthMMMByNumber(const QString& _date_string);

    embeddedmz::CFTPClient m_ftp;
    bool m_connected;

signals:
    void si_connected();
    void si_connectionFailed(QString _err);
    void si_dirContents(QList<network_tools::NetworkFileInfo*> _contents);

};

#endif // THREADABLEFTPCLIENT_H

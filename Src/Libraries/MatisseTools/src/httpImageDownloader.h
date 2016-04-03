#ifndef HTTPFILEDOWNLOADER_H
#define HTTPFILEDOWNLOADER_H

#include <QObject>
#include <QtNetwork/QNetworkAccessManager>
#include <QImage>

class HTTPImageDownloader : public QObject
{
    Q_OBJECT
public:
    explicit HTTPImageDownloader(QObject *parent = 0);

    bool startDownloadOfFile(QString fileUrl_p);

signals:
    void signal_imageReady(QImage image_p);

public slots:

protected slots:
    void slot_receiveReply(QNetworkReply* reply_p);

private:


    QNetworkAccessManager _manager;
};

#endif // HTTPFILEDOWNLOADER_H

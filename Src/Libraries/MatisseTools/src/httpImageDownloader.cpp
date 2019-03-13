#include "httpImageDownloader.h"
#include <QUrl>
#include <QNetworkRequest>
#include <QNetworkReply>
#include <QImage>
#include <QDebug>
#include <QPixmap>

///
/// Class for HTML file downloading
///
///


HTTPImageDownloader::HTTPImageDownloader(QObject *parent) : QObject(parent)
{
    connect(&_manager, SIGNAL(finished(QNetworkReply*)), this, SLOT(slot_receiveReply(QNetworkReply*)));
}

bool HTTPImageDownloader::startDownloadOfFile(QString fileUrl_p)
{
    _manager.get(QNetworkRequest(QUrl(fileUrl_p)));
    return true;
}

void HTTPImageDownloader::slot_receiveReply(QNetworkReply *reply_p)
{

    QByteArray replyData = reply_p->readAll();
    QImage qImage;
    qImage.loadFromData(replyData);

    emit signal_imageReady(qImage);


}


#include "network_file_client.h"

namespace MatisseCommon {

NetworkFileClient::NetworkFileClient() : NetworkClient()
{

}

void NetworkFileClient::connectAction(NetworkAction *_action)
{
    connect(_action, SIGNAL(si_initFileChannel()), SLOT(sl_initFileChannel()));
    connect(_action, SIGNAL(si_upload(QString, QString, bool)), SLOT(sl_upload(QString, QString, bool)));
    connect(_action, SIGNAL(si_download(QString, QString, bool)), SLOT(sl_download(QString, QString, bool)));
    connect(_action, SIGNAL(si_dirContent(QString, FileTypeFilters, QStringList)), SLOT(sl_dirContent(QString, FileTypeFilters, QStringList)));
}

void NetworkFileClient::disconnectAction(NetworkAction *_action)
{
    disconnect(_action, SIGNAL(si_initFileChannel()), this, SLOT(sl_initFileChannel()));
    disconnect(_action, SIGNAL(si_upload(QString, QString, bool)), this, SLOT(sl_upload(QString, QString, bool)));
    disconnect(_action, SIGNAL(si_download(QString, QString, bool)), this, SLOT(sl_download(QString, QString, bool)));
    disconnect(_action, SIGNAL(si_dirContent(QString, FileTypeFilters, QStringList)), this, SLOT(sl_dirContent(QString, FileTypeFilters, QStringList)));
}

} // namespace MatisseCommon

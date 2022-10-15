#include "qftpclient.h"

QFTPClient::QFTPClient(QObject *parent) : QObject(parent)
{
	// move the task object to the thread BEFORE connecting any signal/slots
	m_ftp_client.moveToThread(&m_ftp_thread);

	// connect sig/slot (thread safe)
	connect(this, SIGNAL(si_connectToHost(QString, QString, QString, unsigned)), &m_ftp_client, SLOT(sl_connectToHost(QString, QString, QString, unsigned)));
	connect(this, SIGNAL(si_listDir(QString)), &m_ftp_client, SLOT(sl_listDir(QString)));

	// connect sig/slot (thread safe)
	connect(&m_ftp_client, SIGNAL(si_connected()), this, SLOT(sl_connected()));
	connect(&m_ftp_client, SIGNAL(si_connectionFailed(QString)), this, SLOT(sl_connectionFailed(QString)));
	connect(&m_ftp_client, SIGNAL(si_errorOccured(network_tools::eTransferError, QString)), this, SLOT(sl_errorOccured(network_tools::eTransferError, QString)));
	connect(&m_ftp_client, SIGNAL(si_dirContents(QList<network_tools::NetworkFileInfo*>)), this, SLOT(sl_dirContents(QList<network_tools::NetworkFileInfo*>)));
	connect(&m_ftp_client, SIGNAL(si_progressUpdate(int)), this, SLOT(sl_progressUpdate(int)));
	connect(&m_ftp_client, SIGNAL(si_transferFinished()), this, SLOT(sl_transferFinished()));

	connect(this, SIGNAL(si_uploadFile(QString, QString)), &m_ftp_client, SLOT(sl_uploadFile(QString, QString)));
	connect(this, SIGNAL(si_downloadFile(QString, QString)), &m_ftp_client, SLOT(sl_downloadFile(QString, QString)));
	connect(this, SIGNAL(si_uploadDir(QString, QString, bool)), &m_ftp_client, SLOT(sl_uploadDir(QString, QString, bool)));
	connect(this, SIGNAL(si_downloadDir(QString, QString, bool)), &m_ftp_client, SLOT(sl_downloadDir(QString, QString, bool)));

	m_ftp_thread.start();
}

void QFTPClient::connectToHost(const QString& _host, const QString& _username, const QString& _password, const unsigned& _port)
{
	// asynchronous way to call connect
	emit si_connectToHost(_host, _username, _password, _port);
}

void QFTPClient::listDir(QString _dir)
{
	emit si_listDir(_dir);
}

void QFTPClient::uploadFile(QString _local_file_path, QString _remote_file_path)
{
	emit si_uploadFile(_local_file_path, _remote_file_path);
}

void QFTPClient::downloadFile(QString _remote_file_path, QString _local_file_path)
{
	emit si_downloadFile(_remote_file_path, _local_file_path);
}

void QFTPClient::uploadDir(QString _local_dir_path, QString _remote_dir_path, bool _recursive)
{
	emit si_uploadDir(_local_dir_path, _remote_dir_path, _recursive);
}

void QFTPClient::downloadDir(QString _remote_dir_path, QString _local_dir_path, bool _recursive)
{
	emit si_downloadDir(_remote_dir_path, _local_dir_path,  _recursive);
}

void QFTPClient::stopThread()
{
	m_ftp_thread.quit();
	m_ftp_thread.wait();
}

void QFTPClient::sl_connected()
{
	emit si_connected();
}

void QFTPClient::sl_connectionFailed(QString _err)
{
	emit si_connectionFailed(_err);
}

void QFTPClient::sl_errorOccured(network_tools::eTransferError _error_type, QString _error_msg)
{
	emit si_errorOccured(_error_type, _error_msg);
}

void QFTPClient::sl_dirContents(QList<network_tools::NetworkFileInfo*> _dir_contents)
{
	emit si_dirContents(_dir_contents);
}

void QFTPClient::sl_progressUpdate(int _progress)
{
	emit si_progressUpdate(_progress);
}

void QFTPClient::sl_transferFinished()
{
	emit si_transferFinished();
}
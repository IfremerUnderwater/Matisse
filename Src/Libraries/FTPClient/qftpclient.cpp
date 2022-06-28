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


void QFTPClient::sl_connected()
{
	emit si_connected();
}
void QFTPClient::sl_connectionFailed(QString _err)
{
	emit si_connectionFailed(_err);
}
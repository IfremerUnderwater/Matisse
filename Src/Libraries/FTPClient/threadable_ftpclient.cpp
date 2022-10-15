#include "threadable_ftpclient.h"
#include <QDir>

using namespace embeddedmz;

int ThreadableFTPClient::DLProgressCallback(void* ptr, double dTotalToDownload, double dNowDownloaded, double dTotalToUpload, double dNowUploaded) {

    CFTPClient::ProgressFnStruct* progress_data = static_cast<CFTPClient::ProgressFnStruct*>(ptr);
    ThreadableFTPClient* ftp_client = static_cast<ThreadableFTPClient*>(progress_data->pOwner);

    // ensure that the file to be downloaded is not empty
    // because that would cause a division by zero error later on
    if (dTotalToDownload <= 0.0) return 0;

    // how wide you want the progress meter to be
    const int iTotalDots = 20;
    double dFractionDownloaded = dNowDownloaded / dTotalToDownload;
    // part of the progressmeter that's already "full"
    int iDots = static_cast<int>(round(dFractionDownloaded * iTotalDots));

    // create the "meter"
    int iDot = 0;
    std::cout << static_cast<unsigned>(dFractionDownloaded * 100) << "% [";

    ftp_client->emitProgress(static_cast<int>(dFractionDownloaded * 100));

    // part  that's full already
    for (; iDot < iDots; iDot++) std::cout << "=";

    // remaining part (spaces)
    for (; iDot < iTotalDots; iDot++) std::cout << " ";

    // and back to line begin - do not forget the fflush to avoid output buffering problems!
    std::cout << "]           \r" << std::flush;

    // if you don't return 0, the transfer will be aborted - see the documentation
    return 0;
}

void ThreadableFTPClient::emitProgress(int _progress)
{
    if(m_current_state == DOWNLOADING)
        emit si_progressUpdate(_progress);
}

bool ThreadableFTPClient::isConnectionOk()
{
    std::string strList;

    // test connection by listing home directory
    if (m_ftp.List("~/", strList, false))
    {
        std::cout << "strList = " << strList <<std::endl;
        if (!strList.empty())
            return true;
        else
            return false;
    }
    else
        return false;
}

ThreadableFTPClient::ThreadableFTPClient(QObject *_parent) : QObject(_parent),
m_ftp([](const std::string& _strLogMsg) { std::cout << _strLogMsg << std::endl; }),
m_connected(false)
{

}

void ThreadableFTPClient::sl_connectToHost(QString _host, QString _username, QString _password, unsigned _port)
{
    m_ftp.SetProgressFnCallback(this, ThreadableFTPClient::DLProgressCallback);

    if (m_ftp.InitSession(_host.toStdString(), _port, _username.toStdString(), _password.toStdString()))
    {
        if (isConnectionOk())
        {
            m_connected = true;
            emit si_connected();
            return;
        }

    }
    
    m_ftp.CleanupSession();
    m_connected = false;
    emit si_connectionFailed("Connection Error, wrong credentials or connectivity failure");
}

void ThreadableFTPClient::sl_listDir(QString _dir)
{
    std::string strList;

    // only support linux server
    if(!_dir.endsWith("/"))
        _dir = _dir + "/";

    // get full detail
    m_ftp.List(_dir.toStdString(), strList, false);

    QStringList file_list = QString::fromStdString(strList).split("\r\n");

    QList<network_tools::NetworkFileInfo*> dir_content;
    
    QString current_year = QDateTime::currentDateTime().toString("yyyy");
    
    //Decode remote files' size and time:
    for (int i = 0; i < file_list.size(); i++)
    {
        QString current_file_info = file_list[i];
        if (!current_file_info.isEmpty())
        {
            QString date_to_decode = current_file_info.mid(43, 12);
            QDateTime current_datetime;
            if (date_to_decode.contains(":"))
                current_datetime = QDateTime::fromString(replaceMonthMMMByNumber(date_to_decode) + " " + current_year, "MM dd hh:mm yyyy");
            else
                current_datetime = QDateTime::fromString(replaceMonthMMMByNumber(date_to_decode) + " 12:00", "MM dd  yyyy hh:mm");
            
            // decode file type
            int file_type = current_file_info.mid(14, 1).toInt();
            bool is_dir = file_type == 1 ? false : true;

            // decode file size
            int file_size = current_file_info.mid(31, 11).toInt();

            // get filename
            QString filename = current_file_info.mid(56);

            network_tools::NetworkFileInfo* current_file_info = new network_tools::NetworkFileInfo(filename, is_dir, file_size, current_datetime);
            dir_content.append(current_file_info);

            // print info to debug
            //std::cout << "is_dir = " << is_dir << "\n";
            //std::cout << "filename = " << filename.toStdString() << "\n";
            //std::cout << "date = " << current_datetime.toString("dd-MM-yyyy hh:mm").toStdString() << "\n";

        }
    }

    emit si_dirContents(dir_content);


}

QString ThreadableFTPClient::replaceMonthMMMByNumber(const QString& _date_string)
{
    QString replaced_date_string = _date_string;

    replaced_date_string=replaced_date_string.replace("Jan", "01");
    replaced_date_string = replaced_date_string.replace("Feb", "02");
    replaced_date_string = replaced_date_string.replace("Mar", "03");
    replaced_date_string = replaced_date_string.replace("Apr", "04");
    replaced_date_string = replaced_date_string.replace("May", "05");
    replaced_date_string = replaced_date_string.replace("Jun", "06");
    replaced_date_string = replaced_date_string.replace("Jul", "07");
    replaced_date_string = replaced_date_string.replace("Aug", "08");
    replaced_date_string = replaced_date_string.replace("Sep", "09");
    replaced_date_string = replaced_date_string.replace("Oct", "10");
    replaced_date_string = replaced_date_string.replace("Nov", "11");
    replaced_date_string = replaced_date_string.replace("Dec", "12");

    return replaced_date_string;
}


bool ThreadableFTPClient::sl_uploadFile(QString _local_file_path, QString _remote_file_path)
{
    if (m_connected)
    {
        m_current_state = ThreadableFTPClient::UPLOADING;
        if (!m_ftp.UploadFile(_local_file_path.toStdString(), _remote_file_path.toStdString(), true))
        {
            emit si_errorOccured(network_tools::eTransferError::PERMISSION_DENIED, "Cannot upload file (permission issue or folder in folder not created)");
            return false;
        }
        m_current_state = ThreadableFTPClient::IDLE;
    }
    else
    {
        emit si_errorOccured(network_tools::eTransferError::NO_CONNECTION, "Cannot upload a file while not connected");
        return false;
    }
    return true;

}
void ThreadableFTPClient::sl_downloadFile(QString _remote_file_path, QString _local_file_path)
{
    if (m_connected)
    {
        m_current_state = ThreadableFTPClient::DOWNLOADING;
        if (!m_ftp.DownloadFile(_local_file_path.toStdString(), _remote_file_path.toStdString()))
            emit si_errorOccured(network_tools::eTransferError::PERMISSION_DENIED, "Cannot download (permission issue ?)");
        m_current_state = ThreadableFTPClient::IDLE;
    }
    else
        emit si_errorOccured(network_tools::eTransferError::NO_CONNECTION, "Cannot download a file while not connected");

    emit si_transferFinished();

}
void ThreadableFTPClient::sl_uploadDir(QString _local_dir_path, QString _remote_dir_path, bool _recursive)
{
    QDir local_dir(_local_dir_path);
    QFileInfoList dir_entries = local_dir.entryInfoList(QDir::Filter::Files | QDir::Filter::NoDotAndDotDot);

    bool dir_has_files = !dir_entries.isEmpty();

    if (dir_has_files) {
        m_current_state = ThreadableFTPClient::UPLOADING;
        for (int i = 0; i < dir_entries.size(); i++) {

            QFileInfo entry = dir_entries[i];
            QStringList file_parts = QDir::toNativeSeparators(entry.canonicalFilePath()).split(QDir::separator());

            if (file_parts.size() > 2)
            {
                int progress = round( 100.0*(double)i/(double)dir_entries.size() );
                QString remote_file_path = _remote_dir_path + "/" + file_parts[file_parts.size() - 2] + "/" + entry.fileName(); // only support linux server
                sl_uploadFile(entry.canonicalFilePath(), remote_file_path);
                emit si_progressUpdate(progress);
            }
        }
        emit si_transferFinished();
        m_current_state = ThreadableFTPClient::IDLE;
    }
}
void ThreadableFTPClient::sl_downloadDir(QString _remote_dir_path, QString _local_dir_path, bool _recursive)
{

    if (m_connected)
    {
        m_current_state = ThreadableFTPClient::DOWNLOADING;
        if (!m_ftp.DownloadWildcard(_local_dir_path.toStdString(), _remote_dir_path.toStdString() + "/*"))
            emit si_errorOccured(network_tools::eTransferError::PERMISSION_DENIED, "Cannot download (permission issue ?)");
        m_current_state = ThreadableFTPClient::IDLE;
    }
    else
        emit si_errorOccured(network_tools::eTransferError::NO_CONNECTION, "Cannot download a file while not connected");

    emit si_transferFinished();

}
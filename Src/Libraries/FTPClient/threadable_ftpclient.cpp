#include "threadable_ftpclient.h"
#include <QDir>

using namespace embeddedmz;

ThreadableFTPClient::ThreadableFTPClient(QObject *_parent) : QObject(_parent),
m_ftp([](const std::string& _strLogMsg) { std::cout << _strLogMsg << std::endl; }),
m_connected(false)
{

}

void ThreadableFTPClient::sl_connectToHost(QString _host, QString _username, QString _password, unsigned _port)
{
    if (m_ftp.InitSession(_host.toStdString(), _port, _username.toStdString(), _password.toStdString()))
    {
        m_connected = true;
        emit si_connected();
    }
    else
        emit si_connectionFailed("Internal connection error");
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

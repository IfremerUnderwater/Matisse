#include "nav_file_reader.h"
#include <QFile>
#include <QTextStream>
#include <QDate>
#include <QTime>
#define _USE_MATH_DEFINES
#include <math.h>
#include <cmath>

double DEG2RAD=M_PI/180;


NavFileReader::NavFileReader(QString _file_path):m_file_path(_file_path),m_is_valid(false),
m_lat_interpolator(NULL),
m_lon_interpolator(NULL),
m_depth_interpolator(NULL),
m_alt_interpolator(NULL),
m_roll_interpolator(NULL),
m_pitch_interpolator(NULL),
m_yaw_interpolator(NULL)
{


    if(loadFileToMemory())
        m_is_valid=true;
}

NavFileReader::~NavFileReader()
{
    if(m_lat_interpolator)
        delete m_lat_interpolator;
    if(m_lon_interpolator)
        delete m_lon_interpolator;
    if(m_depth_interpolator)
        delete m_depth_interpolator;
    if(m_alt_interpolator)
        delete m_alt_interpolator;
    if(m_roll_interpolator)
        delete m_roll_interpolator;
    if(m_pitch_interpolator)
        delete m_pitch_interpolator;
    if(m_yaw_interpolator)
        delete m_yaw_interpolator;
}

bool NavFileReader::loadFileToMemory()
{
    m_datetime.clear();
    m_lat.clear();
    m_lon.clear();
    m_depth.clear();
    m_alt.clear();
    m_roll.clear();
    m_pitch.clear();
    m_yaw.clear();

    bool customizable_nav_file = false;
    CustomNavFileInfo custom_nav_info;

    if( !m_file_path.isEmpty() )
    {
        int headerlines = 1;

        QFile file(m_file_path);
        if(!file.open(QIODevice::ReadOnly)) {
            return false;
        }

        QTextStream file_stream(&file);
        int index = 0;

        while(!file_stream.atEnd()) {

            QString line = file_stream.readLine();
            QStringList fields_tab = line.split("\t");
            QStringList fields_com = line.split(",");

            if (index == 0)
            {
                // fill custom nav indexes
                if (line.startsWith("##"))
                {
                    customizable_nav_file = true;
                    line.replace("##", "");
                    QStringList header_fields = line.split(",");

                    for (int i = 0; i < header_fields.size(); i++)
                    {
                        if (header_fields[i].startsWith("date"))
                        {
                            QStringList under_split = header_fields[i].split('_');
                            if (under_split.size() < 2 || (under_split[1]!="yyyy/MM/dd" && under_split[1] != "dd/MM/yyyy" && under_split[1] != "yy/MM/dd" && under_split[1] != "dd/MM/yy"))
                            {
                                file.close();
                                return false;
                            }
                            custom_nav_info.date_format = under_split[1];
                            custom_nav_info.date_index = i;
                        }
                        if (header_fields[i].startsWith("time"))
                        {
                            custom_nav_info.time_index = i;
                        }
                        if (header_fields[i].startsWith("latitude"))
                            custom_nav_info.lat_index = i;
                        if (header_fields[i].startsWith("longitude"))
                            custom_nav_info.lon_index = i;
                        if (header_fields[i].startsWith("depth"))
                            custom_nav_info.depth_index = i;
                        if (header_fields[i].startsWith("heading"))
                            custom_nav_info.heading_index = i;
                        if (header_fields[i].startsWith("roll"))
                            custom_nav_info.roll_index = i;
                        if (header_fields[i].startsWith("pitch"))
                            custom_nav_info.pitch_index = i;
                        if (header_fields[i].startsWith("altitude"))
                            custom_nav_info.alt_index = i;
                    }

                    // cannot process nav without those fields
                    if (custom_nav_info.date_index < 0 || custom_nav_info.time_index < 0 || custom_nav_info.lat_index < 0 || custom_nav_info.lon_index < 0 || custom_nav_info.depth_index < 0)
                    {
                        file.close();
                        return false;
                    }
                    
                }
            }

            if (index >headerlines-1)
            {
                if (customizable_nav_file)
                {
                    std::pair<double, double> pair;

                    QDate date;
                    QString date_format = custom_nav_info.date_format;

                    if (date_format == "yy/MM/dd")
                    {
                        QStringList date_fields = fields_com[custom_nav_info.date_index].split('/');
                        int yy = date_fields[0].toInt();
                        if (yy > 70)
                            yy += 1900;
                        else
                            yy += 2000;
                        fields_com[custom_nav_info.date_index] = QString::number(yy) + "/" + date_fields[1] + "/" + date_fields[2];
                        date_format = "yyyy/MM/dd";
                    }
                    if (date_format == "dd/MM/yy")
                    {
                        QStringList date_fields = fields_com[custom_nav_info.date_index].split('/');
                        int yy = date_fields[2].toInt();
                        if (yy > 70)
                            yy += 1900;
                        else
                            yy += 2000;
                        fields_com[custom_nav_info.date_index] = date_fields[0] + "/" + date_fields[1] + "/" + QString::number(yy);
                        date_format = "dd/MM/yyyy";
                    }

                    date = QDate::fromString(fields_com[custom_nav_info.date_index], date_format);

                    QRegExp time_format_rex("(.+):(.+):(.+).(.+)");
                    QTime time;
                    if (fields_com[custom_nav_info.time_index].contains(time_format_rex))
                    {
                        time = QTime::fromString(fields_com[custom_nav_info.time_index].mid(0, 12), "hh:mm:ss.zzz");
                    }
                    else
                    {
                        time = QTime::fromString(fields_com[custom_nav_info.time_index].mid(0, 8), "hh:mm:ss");
                    }

                    QDateTime date_time(date, time);
                    m_datetime.push_back((double)date_time.toMSecsSinceEpoch());
                    pair.first = (double)date_time.toMSecsSinceEpoch();

                    pair.second = fields_com[custom_nav_info.lat_index].toDouble();
                    m_lat.push_back(pair);

                    pair.second = fields_com[custom_nav_info.lon_index].toDouble();
                    m_lon.push_back(pair);

                    pair.second = fields_com[custom_nav_info.depth_index].toDouble();
                    m_depth.push_back(pair);

                    if (custom_nav_info.alt_index > 0)
                        pair.second = fields_com[custom_nav_info.alt_index].toDouble();
                    else
                        pair.second = 0;
                    m_alt.push_back(pair);

                    if (custom_nav_info.heading_index > 0)
                        pair.second = DEG2RAD * fields_com[custom_nav_info.heading_index].toDouble();
                    else
                        pair.second = 0;
                    m_yaw.push_back(pair);

                    if (custom_nav_info.pitch_index > 0)
                        pair.second = DEG2RAD * fields_com[custom_nav_info.pitch_index].toDouble();
                    else
                        pair.second = 0;
                    m_pitch.push_back(pair);

                    if (custom_nav_info.roll_index > 0)
                        pair.second = DEG2RAD * fields_com[custom_nav_info.roll_index].toDouble();
                    else
                        pair.second = 0;
                    m_roll.push_back(pair);
                }
                else if (fields_tab.size()>=8)
                {

                    std::pair<double, double> pair;

                    QDate date= QDate::fromString(fields_tab[0],"dd/MM/yyyy");
                    QTime time = QTime::fromString(fields_tab[1].mid(0,12),"hh:mm:ss.zzz");
                    if( !time.isValid() )
                    {
                        time = QTime::fromString(fields_tab[1],"hh:mm:ss");
                    }
                    QDateTime date_time(date,time);
                    m_datetime.push_back((double)date_time.toMSecsSinceEpoch());
                    pair.first = (double)date_time.toMSecsSinceEpoch();

                    pair.second = fields_tab[2].toDouble();
                    m_lat.push_back(pair);

                    pair.second = fields_tab[3].toDouble();
                    m_lon.push_back(pair);

                    pair.second = -fields_tab[4].toDouble(); // depth is reversed for phins
                    m_depth.push_back(pair);

                    pair.second = -99.0; // alt not given by this format
                    m_alt.push_back(pair);

                    pair.second = DEG2RAD*fields_tab[5].toDouble();
                    m_yaw.push_back(pair);

                    pair.second = DEG2RAD*fields_tab[6].toDouble();
                    m_pitch.push_back(pair);

                    pair.second = DEG2RAD*fields_tab[7].toDouble();
                    m_roll.push_back(pair);

                }
                else if(fields_com.size()>=8)
                {
                    std::pair<double, double> pair;
                    QRegExp english_date("[0-9]{4}/.+");

                    QDate date;

                    if (fields_com[0].contains(english_date))
                    {
                        date = QDate::fromString(fields_com[0],"yyyy/MM/dd");
                    }else
                    {
                        date = QDate::fromString(fields_com[0],"dd/MM/yyyy");
                    }

                    QRegExp time_format_rex("(.+):(.+):(.+).(.+)");
                    QTime time;
                    if (fields_com[1].contains(time_format_rex))
                    {
                        time = QTime::fromString(fields_com[1],"hh:mm:ss.zzz");
                    }
                    else
                    {
                        time = QTime::fromString(fields_com[1].mid(0,8),"hh:mm:ss");
                    }

                    QDateTime date_time(date,time);
                    m_datetime.push_back((double)date_time.toMSecsSinceEpoch());
                    pair.first = (double)date_time.toMSecsSinceEpoch();

                    pair.second = fields_com[2].toDouble();
                    m_lat.push_back(pair);

                    pair.second = fields_com[3].toDouble();
                    m_lon.push_back(pair);

                    pair.second = fields_com[4].toDouble();
                    m_depth.push_back(pair);

                    pair.second = fields_com[5].toDouble();
                    m_alt.push_back(pair);

                    pair.second = DEG2RAD*fields_com[6].toDouble();
                    m_yaw.push_back(pair);

                    pair.second = 0.0;
                    m_pitch.push_back(pair);

                    pair.second = 0.0;
                    m_roll.push_back(pair);
                }
                else
                {
                    file.close();
                    return false;
                }

            }
            index++;

        }

        file.close();

        m_lat_interpolator = new Interpolator(m_lat);
        m_lon_interpolator = new Interpolator(m_lon);
        m_depth_interpolator = new Interpolator(m_depth);
        m_alt_interpolator = new Interpolator(m_alt);
        m_yaw_interpolator = new AngleInterpolator(m_yaw);
        m_pitch_interpolator = new AngleInterpolator(m_pitch);
        m_roll_interpolator = new AngleInterpolator(m_roll);

        return true;
    }
    else
    {
        return false;
    }
}

bool NavFileReader::isValid() const
{
    return m_is_valid;
}

double NavFileReader::latAtTime(double &_time)
{
    return m_lat_interpolator->findValue(_time);
}

double NavFileReader::lonAtTime(double &_time)
{
    return m_lon_interpolator->findValue(_time);
}

double NavFileReader::depthAtTime(double &_time)
{
    return m_depth_interpolator->findValue(_time);
}

double NavFileReader::altAtTime(double &_time)
{
    return m_alt_interpolator->findValue(_time);
}

double NavFileReader::rollAtTime(double &_time)
{
    return m_roll_interpolator->findValue(_time);
}

double NavFileReader::pitchAtTime(double &_time)
{
    return m_pitch_interpolator->findValue(_time);
}

double NavFileReader::yawAtTime(double &_time)
{
    return m_yaw_interpolator->findValue(_time);
}

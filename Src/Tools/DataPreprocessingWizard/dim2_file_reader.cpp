#include "dim2_file_reader.h"
#include <QFile>
#include <QTextStream>
#include <QDate>
#include <QTime>
#define _USE_MATH_DEFINES
#include <math.h>
#include <cmath>


Dim2FileReader::Dim2FileReader(QString _file_path) :m_file_path(_file_path), m_is_valid(false),
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

Dim2FileReader::~Dim2FileReader()
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

bool Dim2FileReader::loadFileToMemory()
{
    double DEG2RAD = M_PI / 180;

    m_datetime.clear();
    m_lat.clear();
    m_lon.clear();
    m_depth.clear();
    m_alt.clear();
    m_roll.clear();
    m_pitch.clear();
    m_yaw.clear();

    if( !m_file_path.isEmpty() )
    {

        QFile file(m_file_path);
        if(!file.open(QIODevice::ReadOnly)) {
            return false;
        }

        QTextStream file_stream(&file);
        int index = 0;

        while(!file_stream.atEnd()) {

            QString line = file_stream.readLine();
            QStringList fields = line.split(";");


                if(fields.size()>=12)
                {
                    std::pair<double, double> pair;

                    QDate date = QDate::fromString(fields[1],"dd/MM/yyyy");
 
                    QRegExp time_format_rex("(.+):(.+):(.+).(.+)");
                    QTime time = QTime::fromString(fields[2].mid(0,12),"hh:mm:ss.zzz");

                    QDateTime date_time(date,time);
                    m_datetime.push_back((double)date_time.toMSecsSinceEpoch());
                    pair.first = (double)date_time.toMSecsSinceEpoch();

                    pair.second = fields[6].toDouble();
                    m_lat.push_back(pair);

                    pair.second = fields[7].toDouble();
                    m_lon.push_back(pair);

                    pair.second = fields[8].toDouble();
                    m_depth.push_back(pair);

                    pair.second = fields[9].toDouble();
                    m_alt.push_back(pair);

                    pair.second = DEG2RAD*fields[10].toDouble();
                    m_yaw.push_back(pair);

                    pair.second = DEG2RAD*fields[11].toDouble();
                    m_roll.push_back(pair);

                    pair.second = DEG2RAD*fields[12].toDouble();
                    m_pitch.push_back(pair);

                }
                else
                {
                    file.close();
                    return false;
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

bool Dim2FileReader::isValid() const
{
    return m_is_valid;
}

double Dim2FileReader::latAtTime(double &_time)
{
    return m_lat_interpolator->findValue(_time);
}

double Dim2FileReader::lonAtTime(double &_time)
{
    return m_lon_interpolator->findValue(_time);
}

double Dim2FileReader::depthAtTime(double &_time)
{
    return m_depth_interpolator->findValue(_time);
}

double Dim2FileReader::altAtTime(double &_time)
{
    return m_alt_interpolator->findValue(_time);
}

double Dim2FileReader::rollAtTime(double &_time)
{
    return m_roll_interpolator->findValue(_time);
}

double Dim2FileReader::pitchAtTime(double &_time)
{
    return m_pitch_interpolator->findValue(_time);
}

double Dim2FileReader::yawAtTime(double &_time)
{
    return m_yaw_interpolator->findValue(_time);
}

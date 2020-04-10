#ifndef NAV_FILE_READER_H
#define NAV_FILE_READER_H
#include <QString>
#include <vector>
#include "interpolator.h"
#include "angle_interpolator.h"

class NavFileReader
{
public:
    NavFileReader(QString _file_path);
    ~NavFileReader();

    bool loadFileToMemory();
    bool isValid() const;

    double latAtTime(double &_time);
    double lonAtTime(double & _time);
    double depthAtTime(double & _time);
    double altAtTime(double & _time);
    double rollAtTime(double & _time);
    double pitchAtTime(double & _time);
    double yawAtTime(double & _time);

private:
    QString m_file_path;
    std::vector<double> m_datetime;
    std::vector<std::pair<double, double>> m_lat;
    std::vector<std::pair<double, double>> m_lon;
    std::vector<std::pair<double, double>> m_depth;
    std::vector<std::pair<double, double>> m_alt;
    std::vector<std::pair<double, double>> m_roll;
    std::vector<std::pair<double, double>> m_pitch;
    std::vector<std::pair<double, double>> m_yaw;

    Interpolator* m_lat_interpolator;
    Interpolator* m_lon_interpolator;
    Interpolator* m_depth_interpolator;
    Interpolator* m_alt_interpolator;
    AngleInterpolator* m_roll_interpolator;
    AngleInterpolator* m_pitch_interpolator;
    AngleInterpolator* m_yaw_interpolator;

    bool m_is_valid;
};

#endif // NAV_FILE_READER_H

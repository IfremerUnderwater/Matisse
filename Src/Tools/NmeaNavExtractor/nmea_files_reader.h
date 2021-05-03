#ifndef NAV_FILE_READER_H
#define NAV_FILE_READER_H
#include <QStringList>
#include <QFileInfoList>
#include <QString>
#include <vector>
#include "interpolator.h"
#include "angle_interpolator.h"

class NmeaFilesReader
{
public:
    NmeaFilesReader(QFileInfoList _nmea_files_path);
    ~NmeaFilesReader();

    bool loadFilesToMemory();
    bool isValid() const;

    double latAtTime(double &_time);
    double lonAtTime(double & _time);
    double depthAtTime(double & _time);
    double altAtTime(double& _time);
    double rollAtTime(double & _time);
    double pitchAtTime(double & _time);
    double yawAtTime(double & _time);

    double panAtTime(double & _time);
    double tiltAtTime(double & _time);
    double zoomAtTime(double & _time);

    std::vector<double>& getNavDateTime();

private:
    QFileInfoList m_nmea_files_info;

    // Variables
    std::vector<double> m_nav_datetime;
    std::vector<std::pair<double, double>> m_lat;
    std::vector<std::pair<double, double>> m_lon;
    std::vector<std::pair<double, double>> m_depth;
    std::vector<std::pair<double, double>> m_alt;
    std::vector<std::pair<double, double>> m_roll;
    std::vector<std::pair<double, double>> m_pitch;
    std::vector<std::pair<double, double>> m_yaw;

    std::vector<std::pair<double, double>> m_pan;
    std::vector<std::pair<double, double>> m_tilt;
    std::vector<std::pair<double, double>> m_zoom;

    // Interpolators
    Interpolator* m_lat_interpolator;
    Interpolator* m_lon_interpolator;
    Interpolator* m_depth_interpolator;
    Interpolator* m_alt_interpolator;

    AngleInterpolator* m_roll_interpolator;
    AngleInterpolator* m_pitch_interpolator;
    AngleInterpolator* m_yaw_interpolator;

    Interpolator* m_pan_interpolator;
    Interpolator* m_tilt_interpolator;
    Interpolator* m_zoom_interpolator;

    bool m_is_valid;
};

#endif // NAV_FILE_READER_H

#include <QDir>

#include "RTSurveyPlotter.h"

Q_EXPORT_PLUGIN2(RTSurveyPlotter, RTSurveyPlotter)

RTSurveyPlotter::RTSurveyPlotter(QObject *parent):
    RasterProvider(NULL, "RTSurveyPlotter", "", 1)
{
    Q_UNUSED(parent)

    addExpectedParameter("vehic_param", "alt_setpoint");
    addExpectedParameter("cam_param",  "K");
    addExpectedParameter("algo_param", "scale_factor");
    addExpectedParameter("cam_param",  "V_Pose_C");
}

RTSurveyPlotter::~RTSurveyPlotter()
{
    qDebug() << logPrefix() << "Destroy RTSurveyPlotter";
}

bool RTSurveyPlotter::configure()
{
    qDebug() << logPrefix() << "configure";

    // No raster to write, only real time plotting
    _rastersInfo.clear();

    return true;
}

void RTSurveyPlotter::onNewImage(quint32 port, Image &image)
{
    qDebug() << logPrefix() << "Receive image on port " << port;
}

void RTSurveyPlotter::onFlush(quint32 port)
{
    qDebug() << logPrefix() << "Flush on port " << port;
}

bool RTSurveyPlotter::start()
{
    qDebug() << logPrefix() << " inside start";

    qDebug() << logPrefix() << " out start";
    return true;
}

bool RTSurveyPlotter::stop()
{
    return true;
}

QList<QFileInfo> RTSurveyPlotter::rastersInfo()
{
    return _rastersInfo;
}


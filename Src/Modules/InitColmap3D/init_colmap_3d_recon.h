#pragma once

#include "processor.h"
#include <GeographicLib/LocalCartesian.hpp>

namespace matisse {

/**
 * InitColmap3D
 * @brief  Module de début pour implementer un algorithme de traitement dans Matisse
 */
class InitColmap3D : public Processor
{
    Q_OBJECT
    Q_INTERFACES(matisse::Processor)

#if QT_VERSION >= QT_VERSION_CHECK(5, 0, 0)
    Q_PLUGIN_METADATA(IID "InitColmap3D")
#endif

private:
    bool getCameraIntrinsics(double & _focal, double & _ppx, double & _ppy, const double &_width, const double &_height, int &_distortion_model, cv::Mat &_dist_coeff);

    GeographicLib::LocalCartesian m_ltp_proj;

public:
    InitColmap3D();
    ~InitColmap3D();
    virtual bool configure();
    virtual bool start();
    virtual bool stop();
    virtual void onFlush(quint32 _port);
    virtual void onNewImage(quint32 _port, Image &_image);
};

} // namespace matisse


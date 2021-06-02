#ifndef InitMatchModule_H
#define InitMatchModule_H


#include "processor.h"
#include "openMVG/numeric/eigen_alias_definition.hpp"
#include <GeographicLib/LocalCartesian.hpp>

using namespace matisse;

/**
 * Init3DRecon
 * @brief  Module de début pour implementer un algorithme de traitement dans Matisse
 */
class Init3DRecon : public Processor
{
    Q_OBJECT
    Q_INTERFACES(matisse::Processor)

#if QT_VERSION >= QT_VERSION_CHECK(5, 0, 0)
    Q_PLUGIN_METADATA(IID "Init3DRecon")
#endif

private:
    bool getCameraIntrinsics(double & _focal, double & _ppx, double & _ppy, const double &_width, const double &_height, int &_distortion_model, cv::Mat &_dist_coeff);
    std::pair<bool, openMVG::Vec3> getPriorWeights();

    GeographicLib::LocalCartesian m_ltp_proj;

public:
    Init3DRecon();
    ~Init3DRecon();
    virtual bool configure();
    virtual bool start();
    virtual bool stop();
    virtual void onFlush(quint32 port);
    virtual void onNewImage(quint32 port, Image &image);
};

#endif // InitMatchModule_H

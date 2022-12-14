#ifndef MATISSE_MATCHING_3D_H_
#define MATISSE_MATCHING_3D_H_


#include "processor.h"
#include "openMVG/system/progressinterface.hpp"
//#include "third_party/progress/progress.hpp"
#include "SiftGPU.h"
#include "GPUSift_Image_Describer_io.hpp"
#include "opengl_utils.h"
#include "GPUSift_Matcher_Regions.hpp"

#include "H_F_ACRobust.hpp"

#include "GPU_GeometricFilterer.hpp"

namespace matisse {

enum eGeometricModel
{
    FUNDAMENTAL_MATRIX = 0,
    ESSENTIAL_MATRIX = 1,
    HOMOGRAPHY_MATRIX = 2,
    ESSENTIAL_MATRIX_ANGULAR = 3,
    ESSENTIAL_MATRIX_ORTHO = 4,
    ESSENTIAL_MATRIX_UPRIGHT = 5
};

enum ePairMode
{
    PAIR_EXHAUSTIVE = 0,
    PAIR_CONTIGUOUS = 1,
    PAIR_FROM_FILE = 2,
    PAIR_FROM_GPS = 3
};

/**
 * Module1
 * @brief  Exemple de module pour implementer un algorithme de traitement dans Matisse
 */
class Matching3D : public Processor, public openMVG::system::ProgressInterface
{
    Q_OBJECT
    Q_INTERFACES(matisse::Processor)

#if QT_VERSION >= QT_VERSION_CHECK(5, 0, 0)
    Q_PLUGIN_METADATA(IID "Matching3D")
#endif

public:
    Matching3D();
    ~Matching3D();
    virtual bool configure();
    virtual bool start();
    virtual bool stop();
    virtual void onFlush(quint32 _port);
    virtual void onNewImage(quint32 _port, matisse_image::Image &_image);

/** @brief Initializer of the ProgressInterface class
   * @param[in] expected_count The number of step of the process
   * @param[in] msg an optional status message
   * @return void if the progress class can be initialized, else it return false
   **/
    virtual void Restart(const std::uint32_t expected_count, const std::string& msg = {})
    {
      openMVG::system::ProgressInterface::Restart(expected_count,msg);
	  
        if (!msg.empty())
        {
            QString qmsg = "Matching :" + QString::fromStdString(msg).remove('\n').remove('-');
            emit si_userInformation(qmsg);
        }

    } // restart

private:
    bool computeFeatures();
    bool computeMatches(eGeometricModel _geometric_model_to_compute = FUNDAMENTAL_MATRIX);

    bool m_gpu_features;

    /** @brief Function that ... **/
    std::uint32_t operator+=(const std::uint32_t increment) override
    {
		//openMVG::system::ProgressInterface::operator+=(increment);
        emit si_processCompletion(Percent());
		count_ += increment;
        return count_;
    } // display_tic
};

} // namespace matisse

#endif // MATISSE_MATCHING_3D_H_

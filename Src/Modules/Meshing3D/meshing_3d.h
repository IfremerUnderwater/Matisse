#ifndef MATISSE_MESHING_3D_H_
#define MATISSE_MESHING_3D_H_


#include "processor.h"
#include "openMVG/system/progressinterface.hpp"
//#include "third_party/progress/progress.hpp"

namespace matisse {

/**
 * Meshing3D
 * @brief  This module create a 3D Mesh from sparse of dense 3D point cloud (both openMVG and openMVS formats are supported)
 */
class Meshing3D : public matisse::Processor, public openMVG::system::ProgressInterface
{
    Q_OBJECT
    Q_INTERFACES(matisse::Processor)

#if QT_VERSION >= QT_VERSION_CHECK(5, 0, 0)
    Q_PLUGIN_METADATA(IID "Meshing3D")
#endif

public:
    Meshing3D();
    ~Meshing3D();
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
    QString m_source_dir;
    QString m_outdir;
    QString m_out_filename_prefix;

    /** @brief Function that ... **/
    std::uint32_t operator+=(const std::uint32_t increment) override
    {
		//openMVG::system::ProgressInterface::operator+=(increment);
        emit si_processCompletion(Percent());
		count_ += increment;
        return count_;
    } // display_tic

    bool initMeshing();
    bool meshing(QString _mvs_data_file);
};

} // namespace matisse

#endif // MATISSE_MESHING_3D_H_

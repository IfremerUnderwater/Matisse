#ifndef InitMatchModule_H
#define InitMatchModule_H


#include "processor.h"
#include "third_party/progress/progress.hpp"

/**
 * Meshing3D
 * @brief  This module create a 3D Mesh from sparse of dense 3D point cloud (both openMVG and openMVS formats are supported)
 */
class Meshing3D : public MatisseCommon::Processor, public C_Progress
{
    Q_OBJECT
    Q_INTERFACES(MatisseCommon::Processor)

#if QT_VERSION >= QT_VERSION_CHECK(5, 0, 0)
    Q_PLUGIN_METADATA(IID "Meshing3D")
#endif

public:
    Meshing3D();
    ~Meshing3D();
    virtual bool configure();
    virtual bool start();
    virtual bool stop();
    virtual void onFlush(quint32 port);
    virtual void onNewImage(quint32 port, MatisseCommon::Image &image);

    /** @brief Initializer of the C_Progress class
* @param expected_count The number of step of the process
* @param msg updates the status string. Can be empty to keep the last one.
**/
    void restart(unsigned long ulExpected_count, const std::string& msg = std::string()) override
        //  Effects: display appropriate scale
        //  Postconditions: count()==0, expected_count()==expected_count
    {
        C_Progress::restart(ulExpected_count, msg); //-- Initialize the base class
        if (!msg.empty())
        {
            QString qmsg = logPrefix() + QString::fromStdString(msg).remove('\n');
            emit signal_userInformation(QString::fromStdString(msg));
        }

    } // restart

private:
    QString m_source_dir;
    QString m_outdir;
    QString m_out_filename_prefix;

    /** @brief Function that ... **/
    void inc_tic() override
    {
        emit signal_processCompletion((int)(_count / (float)_expected_count * 100 + .5));
    } // display_tic

    bool initMeshing();
    bool meshing(QString _mvs_data_file);
};

#endif // InitMatchModule_H

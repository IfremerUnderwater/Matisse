﻿#ifndef MATISSE_POINT_CLOUD_DENSIFY_H_
#define MATISSE_POINT_CLOUD_DENSIFY_H_


#include "processor.h"
#include "third_party/progress/progress.hpp"

namespace matisse {

/**
 * PointCloudDensify
 * @brief  This module get sparse point cloud and densify it using openMVS librarie
 */
class PointCloudDensify : public matisse::Processor, public C_Progress
{
    Q_OBJECT
    Q_INTERFACES(matisse::Processor)

#if QT_VERSION >= QT_VERSION_CHECK(5, 0, 0)
    Q_PLUGIN_METADATA(IID "PointCloudDensify")
#endif

public:
    PointCloudDensify();
    ~PointCloudDensify();
    virtual bool configure();
    virtual bool start();
    virtual bool stop();
    virtual void onFlush(quint32 _port);
    virtual void onNewImage(quint32 _port, matisse_image::Image &_image);

    /** @brief Initializer of the C_Progress class
* @param _ul_expected_count The number of step of the process
* @param _msg updates the status string. Can be empty to keep the last one.
**/
    void restart(unsigned long _ul_expected_count, const std::string& _msg = std::string()) override
        //  Effects: display appropriate scale
        //  Postconditions: count()==0, expected_count()==expected_count
    {
        C_Progress::restart(_ul_expected_count, _msg); //-- Initialize the base class
        if (!_msg.empty())
        {
            QString qmsg = logPrefix() + QString::fromStdString(_msg).remove('\n');
            emit si_userInformation(QString::fromStdString(_msg));
        }

    } // restart

private:
    QString m_source_dir;
    QString m_outdir;
    QString m_out_filename_prefix;

    /** @brief Function that ... **/
    void inc_tic() override
    {
        emit si_processCompletion((int)(_count / (float)_expected_count * 100 + .5));
    } // display_tic

    bool initDensify();
    bool DensifyPointCloud(QString _scene_dir, QString _scene_file);
};

} // namespace matisse

#endif // MATISSE_POINT_CLOUD_DENSIFY_H_

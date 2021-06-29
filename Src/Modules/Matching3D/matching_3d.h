#ifndef MATISSE_MATCHING_3D_H_
#define MATISSE_MATCHING_3D_H_


#include "processor.h"
#include "third_party/progress/progress.hpp"

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
    PAIR_FROM_FILE = 2
};

/**
 * Module1
 * @brief  Exemple de module pour implementer un algorithme de traitement dans Matisse
 */
class Matching3D : public Processor, public C_Progress
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
            QString qmsg = "Matching :" + QString::fromStdString(_msg).remove('\n').remove('-');
            emit si_userInformation(qmsg);
        }

    } // restart

private:
    bool computeFeatures();
    bool computeMatches(eGeometricModel _geometric_model_to_compute = FUNDAMENTAL_MATRIX);

    /** @brief Function that ... **/
    void inc_tic() override
    {
        emit si_processCompletion((int)(_count / (float)_expected_count * 100 + .5));
    } // display_tic
};

} // namespace matisse

#endif // MATISSE_MATCHING_3D_H_

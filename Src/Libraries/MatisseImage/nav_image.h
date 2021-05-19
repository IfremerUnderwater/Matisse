#ifndef NAVIMAGE_H
#define NAVIMAGE_H



#include <QDateTime>

#include "nav_info.h"
#include "image.h"

namespace MatisseCommon {

///
/// \brief Image associée à une information de navigation
///
class NavImage : public Image
{
public:

    ///
    /// \brief Constructeur de copie
    ///
    /// Le NaviInfo est copié.
    /// \param other
    ///
    NavImage(const NavImage & other);

    ///
    /// \brief Constructeur d'une image à partir de sa structure OpenCV Mat et de son NavInfo
    /// \param id
    /// \param imageData
    /// \param navInfo
    ///
    NavImage(int id, cv::Mat * imageData, NavInfo navInfo);


    void setNavInfo( NavInfo navInfo);

    NavInfo & navInfo();

    ///
    /// \brief Fonction de deboggage
    /// \return
    ///
    virtual QString dumpAttr();

protected:
    NavInfo _navInfo;
};
}

#endif // NAVIMAGE_H

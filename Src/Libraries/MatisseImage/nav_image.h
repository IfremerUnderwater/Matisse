#ifndef MATISSE_IMAGE_NAV_IMAGE_H_
#define MATISSE_IMAGE_NAV_IMAGE_H_

#include <QDateTime>

#include "nav_info.h"
#include "image.h"

namespace matisse_image {

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
    /// \param _other
    ///
    NavImage(const NavImage & _other);

    ///
    /// \brief Constructeur d'une image à partir de sa structure OpenCV Mat et de son NavInfo
    /// \param _id
    /// \param _image_data
    /// \param _nav_info
    ///
    NavImage(int _id, cv::Mat * _image_data, NavInfo _nav_info);


    void setNavInfo( NavInfo _nav_info);

    NavInfo & navInfo();

    ///
    /// \brief Fonction de deboggage
    /// \return
    ///
    virtual QString dumpAttr();

protected:
    NavInfo m_nav_info;
};
}

#endif // MATISSE_IMAGE_NAV_IMAGE_H_

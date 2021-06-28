#include "nav_image.h"

namespace matisse_image {

NavImage::NavImage(const NavImage &_other):Image(_other),
    m_nav_info(_other.m_nav_info)
{
}


NavImage::NavImage(int _id, cv::Mat *_image_data, NavInfo _nav_info):Image(_id, _image_data),
    m_nav_info(_nav_info)
{

}


void NavImage::setNavInfo(NavInfo _nav_info)
{
    m_nav_info = _nav_info;
}

NavInfo &NavImage::navInfo()
{
    return m_nav_info;
}


QString NavImage::dumpAttr()
{
    QString ret(Image::dumpAttr());
    ret += m_nav_info.dump();

    return ret;

}

} // namespace matisse_image

#include "nav_image.h"

namespace matisse_image {

NavImage::NavImage(const NavImage &other):Image(other),
    _navInfo(other._navInfo)
{
}


NavImage::NavImage(int id, cv::Mat *imageData, NavInfo navInfo):Image(id, imageData),
    _navInfo(navInfo)
{

}


void NavImage::setNavInfo(NavInfo navInfo)
{
    _navInfo = navInfo;
}

NavInfo &NavImage::navInfo()
{
    return _navInfo;
}


QString NavImage::dumpAttr()
{
    QString ret(Image::dumpAttr());
    ret += _navInfo.dump();

    return ret;

}

} // namespace matisse_image

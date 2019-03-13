#include "NavImage.h"

using namespace MatisseCommon;


MatisseCommon::NavImage::NavImage(const MatisseCommon::NavImage &other):Image(other),
    _navInfo(other._navInfo)
{
}


MatisseCommon::NavImage::NavImage(int id, cv::Mat *imageData, MatisseCommon::NavInfo navInfo):Image(id, imageData),
    _navInfo(navInfo)
{

}


void MatisseCommon::NavImage::setNavInfo(MatisseCommon::NavInfo navInfo)
{
    _navInfo = navInfo;
}

NavInfo &NavImage::navInfo()
{
    return _navInfo;
}


QString MatisseCommon::NavImage::dumpAttr()
{
    QString ret(Image::dumpAttr());
    ret += _navInfo.dump();

    return ret;

}

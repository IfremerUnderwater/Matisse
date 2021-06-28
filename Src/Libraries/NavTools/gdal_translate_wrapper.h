#ifndef NAV_TOOLS_GDAL_TRANSLATE_WRAPPER_H_
#define NAV_TOOLS_GDAL_TRANSLATE_WRAPPER_H_


#include <QString>

namespace nav_tools {

class GdalTranslateWrapper
{
public:
    GdalTranslateWrapper();
    void geoReferenceFile(QString _input_file, QString _outpout_file, QString _gdal_options);
};

}

#endif // NAV_TOOLS_GDAL_TRANSLATE_WRAPPER_H_

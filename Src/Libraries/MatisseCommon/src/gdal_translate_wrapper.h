#ifndef GDAL_TRANSLATE_WRAPPER_H
#define GDAL_TRANSLATE_WRAPPER_H

#include "libmatissecommon_global.h"
#include <QString>

class LIBMATISSECOMMONSHARED_EXPORT GdalTranslateWrapper
{
public:
    GdalTranslateWrapper();
    void geoReferenceFile(QString _input_file, QString _outpout_file, QString _gdal_options);
};

#endif // GDAL_TRANSLATE_WRAPPER_H

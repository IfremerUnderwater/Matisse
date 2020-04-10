#include "gdal_translate_wrapper.h"
#include <QProcess>
#include <QDebug>

GdalTranslateWrapper::GdalTranslateWrapper()
{

}

GdalTranslateWrapper::geoReferenceFile(QString _input_file, QString _outpout_file, QString _gdal_options)
{
    QProcess gdal_translate_proc;
    //gdal_translate_proc.setWorkingDirectory(m_root_dirname_str);

#ifdef _WIN32
    gdal_translate_proc.start("gdal_translate.exe "+_gdal_options+" "+_input_file+" "+_outpout_file);
#else
    gdal_translate_proc.start("gdal_translate "+_gdal_options+" "+_input_file+" "+_outpout_file);
#endif

    // run process
    while(gdal_translate_proc.waitForReadyRead(-1))
    {
        QString output = gdal_translate_proc.readAllStandardOutput() + gdal_translate_proc.readAllStandardError();
        qDebug() << output;
    }
}

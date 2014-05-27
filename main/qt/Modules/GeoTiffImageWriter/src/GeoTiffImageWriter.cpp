#include <QDir>

#include "GeoTiffImageWriter.h"

Q_EXPORT_PLUGIN2(GeoTiffImageWriter, GeoTiffImageWriter)

GeoTiffImageWriter::GeoTiffImageWriter(QObject *parent):
    RasterProvider(NULL, "GeoTiffImageWriter", "", 1)
{
    addExpectedParameter("dataset_param", "output_dir");
    addExpectedParameter("dataset_param", "dataset_dir");
    addExpectedParameter("dataset_param", "output_filename");
}

GeoTiffImageWriter::~GeoTiffImageWriter()
{

}

bool GeoTiffImageWriter::configure()
{
    qDebug() << logPrefix() << "configure";

    QString datasetDirnameStr = _matisseParameters->getStringParamValue("dataset_param", "dataset_dir");
    QString outputDirnameStr = _matisseParameters->getStringParamValue("dataset_param", "output_dir");
    QString outputFilename = _matisseParameters->getStringParamValue("dataset_param", "output_filename");

    if (datasetDirnameStr.isEmpty()
     || outputDirnameStr.isEmpty()
     || outputFilename.isEmpty())
        return false;


    QFileInfo outputDirInfo(outputDirnameStr);
    QFileInfo datasetDirInfo(datasetDirnameStr);

    bool isRelativeDir = outputDirInfo.isRelative();

    if (isRelativeDir) {
        outputDirnameStr = QDir::cleanPath( datasetDirInfo.absoluteFilePath() + QDir::separator() + outputDirnameStr);
    }
    _outputFileInfo.setFile(QDir(outputDirnameStr), outputFilename);

    return true;
}

void GeoTiffImageWriter::onNewImage(quint32 port, Image &image)
{
    qDebug() << logPrefix() << "Receive image on port " << port;
}

void GeoTiffImageWriter::onFlush(quint32 port)
{
    qDebug() << logPrefix() << "Flush on port " << port;
}

bool GeoTiffImageWriter::start()
{
    qDebug() << logPrefix() << " inside start";

    qDebug() << logPrefix() << " out start";
    return true;
}

bool GeoTiffImageWriter::stop()
{
    return true;
}

QFileInfo GeoTiffImageWriter::rasterInfo()
{
    return _outputFileInfo;
}


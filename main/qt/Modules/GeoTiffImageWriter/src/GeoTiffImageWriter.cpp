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

bool GeoTiffImageWriter::configure(Context *context, MatisseParameters *mosaicParameters)
{
    qDebug() << logPrefix() << "configure";
    QString datasetDirnameStr = mosaicParameters->getStringParamValue("dataset_param", "dataset_dir");
    QString outputDirnameStr = mosaicParameters->getStringParamValue("dataset_param", "output_dir");
    QString outputFilename = mosaicParameters->getStringParamValue("dataset_param", "output_filename");

    if (outputDirnameStr.isEmpty()
     || datasetDirnameStr.isEmpty()
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

}

void GeoTiffImageWriter::start()
{
    qDebug() << logPrefix() << " inside start";

    qDebug() << logPrefix() << " out start";
}

void GeoTiffImageWriter::stop()
{

}

QFileInfo GeoTiffImageWriter::rasterInfo()
{
    return _outputFileInfo;
}


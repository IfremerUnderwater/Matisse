#include <QDir>

#include "GeoTiffImageWriter.h"

Q_EXPORT_PLUGIN2(GeoTiffImageWriter, GeoTiffImageWriter)

GeoTiffImageWriter::GeoTiffImageWriter(QObject *parent):
    RasterProvider(NULL, "GeoTiffImageWriter", "", 1)
{
    addExpectedParameter("dataset_param", "output_dir");
    addExpectedParameter("dataset_param", "relative_path");
    addExpectedParameter("dataset_param", "output_filename");
}

GeoTiffImageWriter::~GeoTiffImageWriter()
{

}

bool GeoTiffImageWriter::configure(Context *context, MatisseParameters *mosaicParameters)
{
    qDebug() << logPrefix() << "configure";
    QFileInfo* pXmlFileInfo=mosaicParameters->getXmlFileInfo();
    QString outputDirnameStr = mosaicParameters->getStringParamValue("dataset_param", "output_dir");

    bool isOk = false;
    bool isRelativeDir = mosaicParameters->getBoolParamValue("dataset_param", "relative_path", isOk);

    // TODO Améliorer le check
    if (outputDirnameStr.isEmpty() || pXmlFileInfo==NULL)
        return false;

    // TODO Use Absolute if path is relative?
    if (isRelativeDir) {
        outputDirnameStr = QDir::cleanPath( pXmlFileInfo->absolutePath() + QDir::separator() + outputDirnameStr);
    }
    qDebug()<< "outputDirnameStr: "  << outputDirnameStr;

    QString outputFilename = mosaicParameters->getStringParamValue("dataset_param", "output_filename");

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


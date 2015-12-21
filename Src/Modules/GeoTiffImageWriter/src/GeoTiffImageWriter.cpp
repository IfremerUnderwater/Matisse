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
    qDebug() << logPrefix() << "Destroy GeoTiffImageWriter";
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
    QFileInfo outputFileInfo;

    bool isRelativeDir = outputDirInfo.isRelative();

    if (isRelativeDir) {
        outputDirnameStr = QDir::cleanPath( datasetDirInfo.absoluteFilePath() + QDir::separator() + outputDirnameStr);
    }
    outputFileInfo.setFile(QDir(outputDirnameStr), outputFilename);
    _rastersInfo.clear();
    _rastersInfo << outputFileInfo;

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

QList<QFileInfo> GeoTiffImageWriter::rastersInfo()
{
    return _rastersInfo;
}


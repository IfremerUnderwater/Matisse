#include <QDir>

#include "reconstructioncontext.h"
#include "ReconstructionLister.h"

#if QT_VERSION < QT_VERSION_CHECK(5, 0, 0)
Q_EXPORT_PLUGIN2(ReconstructionLister, ReconstructionLister)
#endif

ReconstructionLister::ReconstructionLister(QObject *parent):
    RasterProvider(NULL, "ReconstructionLister", "", 1)
{
    Q_UNUSED(parent)
    addExpectedParameter("dataset_param", "output_dir");
    addExpectedParameter("dataset_param", "dataset_dir");
    addExpectedParameter("dataset_param", "output_filename");
}

ReconstructionLister::~ReconstructionLister()
{
    //qDebug() << logPrefix() << "Destroy ReconstructionLister";
}

bool ReconstructionLister::configure()
{
    return true;
}

void ReconstructionLister::onNewImage(quint32 port, Image &image)
{
    Q_UNUSED(image)
    qDebug() << logPrefix() << "Receive image on port " << port;
}

void ReconstructionLister::onFlush(quint32 port)
{
    qDebug() << logPrefix() << "Flush on port " << port;

    /*_rastersInfo.clear();

    static const QString SEP = QDir::separator();

    // Dir checks
    QString rootDirnameStr = _matisseParameters->getStringParamValue("dataset_param", "dataset_dir");

    QString outDirnameStr = _matisseParameters->getStringParamValue("dataset_param", "output_dir");
    if(outDirnameStr.isEmpty())
        outDirnameStr = "outReconstruction";

    QString completeOutPath = rootDirnameStr + SEP + outDirnameStr;

    QString fileNamePrefixStr = _matisseParameters->getStringParamValue("dataset_param", "output_filename");

    // Get context
    QVariant *object = _context->getObject("reconstruction_context");
    reconstructionContext * rc=NULL;
    if (object)
        rc = object->value<reconstructionContext*>();

    for(unsigned int i=0; i<rc->components_ids.size(); i++)
    {
        QDir outPathDir(QString("%1_%2").arg(completeOutPath).arg(rc->components_ids[i]));
        _rastersInfo << outPathDir.absoluteFilePath(QString("%1_%2_texrecon.obj").arg(fileNamePrefixStr).arg(rc->components_ids[i]));
    }*/
}

bool ReconstructionLister::start()
{
    qDebug() << logPrefix() << " inside start";

    qDebug() << logPrefix() << " out start";
    return true;
}

bool ReconstructionLister::stop()
{

    _rastersInfo.clear();

    static const QString SEP = QDir::separator();

    // Dir checks
    QString rootDirnameStr = _matisseParameters->getStringParamValue("dataset_param", "dataset_dir");

    QString outDirnameStr = _matisseParameters->getStringParamValue("dataset_param", "output_dir");
    if(outDirnameStr.isEmpty())
        outDirnameStr = "outReconstruction";

    QString completeOutPath = rootDirnameStr + SEP + outDirnameStr;

    QString fileNamePrefixStr = _matisseParameters->getStringParamValue("dataset_param", "output_filename");

    // Get context
    QVariant *object = _context->getObject("reconstruction_context");
    reconstructionContext * rc=NULL;
    if (object)
    {
        rc = object->value<reconstructionContext*>();

        for (unsigned int i = 0; i < rc->components_ids.size(); i++)
        {
            QDir outPathDir(QString("%1_%2").arg(completeOutPath).arg(rc->components_ids[i]));
            _rastersInfo << outPathDir.absoluteFilePath(QString("%1_%2%3.obj").arg(fileNamePrefixStr).arg(rc->components_ids[i]).arg(rc->out_file_suffix));
        }

    }

    return true;
}

QList<QFileInfo> ReconstructionLister::rastersInfo()
{
    return _rastersInfo;
}


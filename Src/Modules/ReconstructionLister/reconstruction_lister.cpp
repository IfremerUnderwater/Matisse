#include <QDir>

#include "reconstruction_context.h"
#include "ReconstructionLister.h"

#if QT_VERSION < QT_VERSION_CHECK(5, 0, 0)
Q_EXPORT_PLUGIN2(ReconstructionLister, ReconstructionLister)
#endif

ReconstructionLister::ReconstructionLister(QObject *parent):
    OutputDataWriter(NULL, "ReconstructionLister", "", 1)
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

    _rastersInfo.clear();

    static const QString SEP = QDir::separator();

    // Dir checks
    QString temp_out_dir = absoluteOutputTempDir() + QDir::separator() + "ModelPart";
    QString out_dir = absoluteOutputDir();
    QDir q_out_dir(out_dir);

    QString fileNamePrefixStr = _matisseParameters->getStringParamValue("dataset_param", "output_filename");

    // Get context
    QVariant* object = _context->getObject("reconstruction_context");
    reconstructionContext* rc = NULL;
    if (object)
    {
        rc = object->value<reconstructionContext*>();

        QString out_filename_prefix = _matisseParameters->getStringParamValue("dataset_param", "output_filename");

        for (unsigned int i = 0; i < rc->components_ids.size(); i++)
        {
            QDir temp_out_recons_dir(QString("%1_%2").arg(temp_out_dir).arg(rc->components_ids[i]));
            QString temp = temp_out_recons_dir.path();
            QStringList recons_files = temp_out_recons_dir.entryList(QStringList() << fileNamePrefixStr+"*.png" << fileNamePrefixStr + "*.kml" << fileNamePrefixStr + "*.mtl" << fileNamePrefixStr + "*.obj" << fileNamePrefixStr + "*mesh.ply", QDir::Files);

            for (unsigned int j = 0; j < recons_files.size(); j++)
            {
                QFile current_file(temp_out_recons_dir.absoluteFilePath(recons_files[j]));
                current_file.rename(out_dir + QDir::separator() + recons_files[j]);
            }

            _rastersInfo << q_out_dir.absoluteFilePath(QString("%1_%2%3.obj").arg(fileNamePrefixStr).arg(rc->components_ids[i]).arg(rc->out_file_suffix));

        }

    }

 
}

bool ReconstructionLister::start()
{
    return true;
}

bool ReconstructionLister::stop()
{



    return true;
}

QList<QFileInfo> ReconstructionLister::rastersInfo()
{
    return _rastersInfo;
}


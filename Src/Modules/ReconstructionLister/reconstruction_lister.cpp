#include <QDir>

#include "reconstruction_context.h"
#include "reconstruction_lister.h"

#if QT_VERSION < QT_VERSION_CHECK(5, 0, 0)
Q_EXPORT_PLUGIN2(ReconstructionLister, ReconstructionLister)
#endif

namespace matisse {

ReconstructionLister::ReconstructionLister(QObject *_parent):
    OutputDataWriter(NULL, "ReconstructionLister", "", 1)
{
    Q_UNUSED(_parent)
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

void ReconstructionLister::onNewImage(quint32 _port, Image &_image)
{
    Q_UNUSED(_image)
    qDebug() << logPrefix() << "Receive image on port " << _port;
}

void ReconstructionLister::onFlush(quint32 _port)
{

    m_rasters_info.clear();

    static const QString SEP = QDir::separator();

    // Dir checks
    QString temp_out_dir = absoluteOutputTempDir() + QDir::separator() + "ModelPart";
    QString out_dir = absoluteOutputDir();
    QDir q_out_dir(out_dir);

    QString fileNamePrefixStr = m_matisse_parameters->getStringParamValue("dataset_param", "output_filename");

    // Get context
    QVariant* object = m_context->getObject("reconstruction_context");
    reconstructionContext* rc = NULL;
    if (object)
    {
        rc = object->value<reconstructionContext*>();

        QString out_filename_prefix = m_matisse_parameters->getStringParamValue("dataset_param", "output_filename");

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

            m_rasters_info << q_out_dir.absoluteFilePath(QString("%1_%2%3.obj").arg(fileNamePrefixStr).arg(rc->components_ids[i]).arg(rc->out_file_suffix));

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
    return m_rasters_info;
}

} // namespace matisse


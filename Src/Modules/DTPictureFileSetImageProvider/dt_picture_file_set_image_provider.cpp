#include <QDir>

#include "dt_picture_file_set_image_provider.h"
#include "file_utils.h"

#if QT_VERSION < QT_VERSION_CHECK(5, 0, 0)
Q_EXPORT_PLUGIN2(DTPictureFileSetImageProvider, DTPictureFileSetImageProvider)
#endif
using namespace nav_tools;
using namespace system_tools;

namespace matisse {

DTPictureFileSetImageProvider::DTPictureFileSetImageProvider(QObject *_parent):
    InputDataProvider(NULL, "DTPictureFileSetImageProvider", "", 1),
    m_picture_file_set(NULL),
    m_dim2_file_reader(NULL)
{
    Q_UNUSED(_parent)
    m_image_set = new ImageSet();
    addExpectedParameter("dataset_param", "dataset_dir");
    addExpectedParameter("dataset_param", "navFile");
    addExpectedParameter("algo_param", "First_processed_image");
    addExpectedParameter("algo_param", "Last_processed_image");
    addExpectedParameter("algo_param", "step_im");
    addExpectedParameter("algo_param", "scale_factor");

}

DTPictureFileSetImageProvider::~DTPictureFileSetImageProvider()
{

}



ImageSet * DTPictureFileSetImageProvider::imageSet(quint16 _port)
{
    Q_UNUSED(_port)
    return m_image_set;
}

bool DTPictureFileSetImageProvider::configure()
{
    qDebug() << logPrefix() << "DTPictureFileSetImageProvider configure";

    QString root_dirname_str = m_matisse_parameters->getStringParamValue("dataset_param", "dataset_dir");

    bool is_ok = false;
    QString nav_file_str = m_matisse_parameters->getStringParamValue("dataset_param", "navFile");
    quint32 first_image_index = m_matisse_parameters->getIntParamValue("algo_param", "First_processed_image", is_ok);
    if (!is_ok) {
        first_image_index = 1;
    }
    quint32 last_image_index = m_matisse_parameters->getIntParamValue("algo_param", "Last_processed_image", is_ok);
    if (!is_ok) {
        last_image_index = InfInt;
    }
    quint32 step_index = m_matisse_parameters->getIntParamValue("algo_param", "step_im", is_ok);
    if (!is_ok) {
        step_index = 1;
    }

    /* Resolve UNIX paths ('~/...') for remote job execution */
    root_dirname_str = FileUtils::resolveUnixPath(root_dirname_str);
    nav_file_str = FileUtils::resolveUnixPath(nav_file_str);

    // TODO Améliorer le check
    if (root_dirname_str.isEmpty() || nav_file_str.isEmpty())
        return false;


    qDebug()<< "lastImageIndex: "  << last_image_index;
    qDebug()<< "rootDirnameStr: "  << root_dirname_str;
    qDebug()<< "navFileStr: "  << nav_file_str;

    QFileInfo file_info(root_dirname_str);
    if (!file_info.exists()) {
        qDebug() << "Erreur rootDirName";
        return false;
    }

    file_info.setFile(nav_file_str);
    // si le fichier est en absolu, on ignore l'aspect relatif...
    if (!file_info.isAbsolute()) {
        nav_file_str.prepend(root_dirname_str + QDir::separator());
    }
    file_info.setFile(nav_file_str);

    if (!file_info.exists()) {
        qDebug() << "Erreur navFileStr" << file_info.absoluteFilePath();
        return false;
    }

    m_picture_file_set = new PictureFileSet(root_dirname_str);
    // Le dive number est mis à 0 car lu dans le fichier...
    // on pourrait (normalement...) le lire dans le nom du répertoire...
    m_dim2_file_reader = new Dim2FileReader(nav_file_str, first_image_index, last_image_index, step_index);


    qDebug() << "navFileStr, firstImageIndex, lastImageIndex, stepIndex:" << nav_file_str << first_image_index << last_image_index << step_index;

    return true;

}

bool DTPictureFileSetImageProvider::start()
{
    qDebug() << logPrefix() << " inside start";

    bool ok;
    double scale_factor = m_matisse_parameters->getDoubleParamValue("algo_param", "scale_factor", ok);

    emit si_processCompletion(0);
    emit si_userInformation("Building image set...");

    for(int i=0; i<m_dim2_file_reader->getNumberOfImages(); i++) {

         QString filename = m_dim2_file_reader->getImageFilename(i);
//         qDebug() << logPrefix() << " load image " << filename;
         QFileInfo file_info(m_picture_file_set ->rootDirname(), filename);

         if (file_info.exists() && file_info.isReadable()) {
             QString file_format = m_dim2_file_reader->getImageFormat(i);
             QString file_source = m_dim2_file_reader->getImageSource(i);
             NavInfo nav_info = m_dim2_file_reader->getNavInfo(i);
             FileImage * new_image = new FileImage(m_picture_file_set, filename, file_source, file_format, i, nav_info);
             new_image->setScaleFactor(scale_factor);
             m_image_set->addImage(new_image);
         }

         double progress_ratio = i/m_dim2_file_reader->getNumberOfImages();
         quint8 progress = progress_ratio * 100;
         emit si_processCompletion(progress);
    }
    emit si_processCompletion(100);

    m_image_set->flush();

    qDebug() << logPrefix() << " out start";
    return true;
}

bool DTPictureFileSetImageProvider::stop()
{
    m_image_set->clear();
    return true;
}

} // namespace matisse


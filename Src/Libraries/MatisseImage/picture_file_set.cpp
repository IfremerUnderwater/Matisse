#include "picture_file_set.h"

namespace matisse_image {

PictureFileSet::PictureFileSet(QString _root_dir_name, QString _dim2_file_name, bool _writable):
    m_root_dir_name(_root_dir_name),
    m_dim2_file_name(_dim2_file_name),
    m_writable(_writable)
{

}

QString PictureFileSet::rootDirname()
{
    return m_root_dir_name;
}

QString PictureFileSet::dim2Filename()
{
    return m_dim2_file_name;
}


bool PictureFileSet::isValid()
{
    // Pour l'instant, on suppose que le fichier est dans le répertoire...
    return dim2FileIsValid();
}

bool PictureFileSet::rootDirnameIsValid()
{
    if (m_root_dir_name.isEmpty()) {
        return false;
    }
    QFileInfo info(m_root_dir_name);
    bool ret = info.exists() && info.isDir() && info.isReadable();
    if (m_writable) {
        ret = info.isWritable() && ret;
    }

    return ret;
}

bool PictureFileSet::dim2FileIsValid()
{
    if (m_dim2_file_name.isEmpty()) {
        return false;
    }

    bool ret = rootDirnameIsValid();
    if (ret) {
        QString filename = m_root_dir_name + "/" + m_dim2_file_name;
        QFileInfo info(filename);
        if (!info.exists()) {
            if (m_writable) {
                QFile newFile(filename);
                ret = newFile.open(QIODevice::ReadWrite);
                newFile.close();
            } else {
                ret = false;
            }
        } else {
            ret = info.isFile() && info.isReadable();
            if (m_writable) {
                ret = info.isWritable() && ret;
            }
        }
    }

    return ret;
}

//QSharedPointer<PictureFileSet> PictureFileSet::getNewInstance()
//{
//    return QSharedPointer<PictureFileSet>(new PictureFileSet());
//}

} // namespace matisse_image

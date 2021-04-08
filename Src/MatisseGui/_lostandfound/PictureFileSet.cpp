#include "PictureFileSet.h"

PictureFileSet::PictureFileSet(QString rootDirname, QString dim2Filename, bool writable):_rootDirname(rootDirname),
    _dim2Filename(dim2Filename),
    _writable(writable)
{

}

bool PictureFileSet::setRootDirname(QString rootDirname)
{
    _rootDirname = rootDirname;
    return rootDirnameIsValid();
}

QString PictureFileSet::getRootDirname()
{
    return _rootDirname;
}

QString PictureFileSet::getDim2Filename()
{
    return _dim2Filename;
}

bool PictureFileSet::setDim2Filename(QString filename)
{
    _dim2Filename = filename;
    return dim2FileIsValid();
}

bool PictureFileSet::isValid()
{
    // Pour l'instant, on suppose que le fichier est dans le répertoire...
    return dim2FileIsValid();
}

bool PictureFileSet::rootDirnameIsValid()
{
    if (_rootDirname.isEmpty()) {
        return false;
    }
    QFileInfo info(_rootDirname);
    bool ret = info.exists() && info.isDir() && info.isReadable();
    if (_writable) {
        ret = info.isWritable() && ret;
    }

    return ret;
}

bool PictureFileSet::dim2FileIsValid()
{
    if (_dim2Filename.isEmpty()) {
        return false;
    }

    bool ret = rootDirnameIsValid();
    if (ret) {
        QString filename = _rootDirname + "/" + _dim2Filename;
        QFileInfo info(filename);
        if (!info.exists()) {
            if (_writable) {
                QFile newFile(filename);
                ret = newFile.open(QIODevice::ReadWrite);
                newFile.close();
            } else {
                ret = false;
            }
        } else {
            ret = info.isFile() && info.isReadable();
            if (_writable) {
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

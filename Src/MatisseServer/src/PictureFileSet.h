#ifndef PICTUREFILESET_H
#define PICTUREFILESET_H

#include <QString>
#include <QDir>
#include <QFileInfo>
#include <QSharedPointer>

#include "InputSource.h"

class PictureFileSet : public InputSource
{
public:
    PictureFileSet(QString rootDirname = "", QString dim2Filename = "", bool writable = false);

    bool setRootDirname(QString rootDirname);
    bool setDim2Filename(QString filename);

    QString getRootDirname();
    QString getDim2Filename();
    virtual bool isValid();
    bool rootDirnameIsValid();
    bool dim2FileIsValid();

//    static QSharedPointer<PictureFileSet> getNewInstance();

private:
    QString _rootDirname;
    QString _dim2Filename;
    bool _writable;
};

#endif // PICTUREFILESET_H

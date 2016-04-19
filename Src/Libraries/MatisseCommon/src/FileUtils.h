#ifndef Tools_H
#define Tools_H

#include <QtDebug>
#include <QFile>
#include <QDir>

namespace MatisseCommon {

class FileUtils
{
public:
    static bool removeDir(const QString &dirName);
};
}
#endif // Tools_H

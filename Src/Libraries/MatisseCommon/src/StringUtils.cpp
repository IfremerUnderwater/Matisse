#include "StringUtils.h"


QString StringUtils::substitutePlaceHolders(QString source, QMap<QString, QString> properties, QString placeHolderPattern)
{
    QString resolvedPhPattern = placeHolderPattern.arg(PROPERTY_NAME_PATTERN);
    QRegExp phRex(resolvedPhPattern);

    QString current(source);

//    qDebug() << "Source string with placeholders:\n" << source;

    int index = phRex.indexIn(current);

    while(index >= 0) {
        QString phKey = phRex.cap(1);

        if (properties.contains(phKey)) {
            qDebug() << QString("Subsituting placeholder %1").arg(phKey);
            QString value = properties.value(phKey);
            int phSize = phRex.matchedLength();
            current.replace(index, phSize, value);
        } else {
            qWarning() << QString("Unknown placeholder %1").arg(phKey);
        }

        index = phRex.indexIn(current, index + 1); // index + 1 to avoid looping on unknown placeholders
    }

    return current;
}

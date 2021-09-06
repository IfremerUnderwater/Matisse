#include "string_utils.h"

namespace system_tools {

QString StringUtils::substitutePlaceHolders(QString _source, QMap<QString, QString> _properties, QString _place_holder_pattern)
{
    QString resolved_ph_pattern = _place_holder_pattern.arg(PROPERTY_NAME_PATTERN);
    QRegExp ph_rex(resolved_ph_pattern);

    QString current(_source);

//    qDebug() << "Source string with placeholders:\n" << source;

    int index = ph_rex.indexIn(current);

    while(index >= 0) {
        QString phKey = ph_rex.cap(1);

        if (_properties.contains(phKey)) {
            //qDebug() << QString("Subsituting placeholder %1").arg(phKey);
            QString value = _properties.value(phKey);
            int ph_size = ph_rex.matchedLength();
            current.replace(index, ph_size, value);
        } else {
            qWarning() << QString("Unknown placeholder %1").arg(phKey);
        }

        index = ph_rex.indexIn(current, index + 1); // index + 1 to avoid looping on unknown placeholders
    }

    return current;
}

} // namespace system_tools

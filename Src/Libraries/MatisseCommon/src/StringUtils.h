#ifndef STRINGUTILS_H
#define STRINGUTILS_H

#include <QString>
#include <QRegExp>
#include <QMap>
#include <QtDebug>

#define PROPERTY_NAME_PATTERN           "\\w+(?:\\.\\w+)*"
#define PLACEHOLDER_PATTERN_CSS         "<%(%1)%>"          // Placeholder pattern compatible with CSS syntax
#define PLACEHOLDER_PATTERN_XML_1       "\\{%(%1)%\\}"          // Placeholder pattern compatible with XML syntax
#define PLACEHOLDER_PATTERN_XML_2       "\\{&(%1)&\\}"          // Placeholder pattern compatible with XML syntax (for embedding in pattern #1)
#define PLACEHOLDER_PATTERN_DEFAULT     PLACEHOLDER_PATTERN_CSS

class StringUtils
{
public:
    static QString substitutePlaceHolders(QString source, QMap<QString,QString> properties,
                                          QString placeHolderPattern = QString(PLACEHOLDER_PATTERN_DEFAULT));
};

#endif // STRINGUTILS_H

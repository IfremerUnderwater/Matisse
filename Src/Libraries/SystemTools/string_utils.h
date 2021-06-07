#ifndef SYSTEM_TOOLS_STRING_UTILS_H_
#define SYSTEM_TOOLS_STRING_UTILS_H_



#include <QString>
#include <QRegExp>
#include <QMap>
#include <QtDebug>

#define PROPERTY_NAME_PATTERN           "\\w+(?:\\.\\w+)*"
#define PLACEHOLDER_PATTERN_CSS         "<%(%1)%>"          // Placeholder pattern compatible with CSS syntax
#define PLACEHOLDER_PATTERN_XML_1       "\\{%(%1)%\\}"          // Placeholder pattern compatible with XML syntax
#define PLACEHOLDER_PATTERN_XML_2       "\\{&(%1)&\\}"          // Placeholder pattern compatible with XML syntax (for embedding in pattern #1)
#define PLACEHOLDER_PATTERN_DEFAULT     PLACEHOLDER_PATTERN_CSS

namespace system_tools {

class StringUtils
{
public:
    static QString substitutePlaceHolders(QString _source, QMap<QString,QString> _properties,
                                          QString _place_holder_pattern = QString(PLACEHOLDER_PATTERN_DEFAULT));
};

} // namespace system_tools

#endif // SYSTEM_TOOLS_STRING_UTILS_H_

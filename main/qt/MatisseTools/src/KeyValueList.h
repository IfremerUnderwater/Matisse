#ifndef KEYVALUELIST_H
#define KEYVALUELIST_H

#include <QPair>
#include <QList>
#include <QString>
#include <QStringList>

namespace MatisseTools {
class KeyValueList
{
public:
    KeyValueList();
    KeyValueList(KeyValueList const &other);
    bool append(QString key, QString value = "");
    void set(QString key, QString value);
    bool insert(QString key, QString value, int before = -1);
    int getSize();
    void clear();
    QStringList getKeys();
    QStringList getValues();
    QString getValue(QString key);

private:
    QStringList _keys;
    QStringList _values;
};
}

#endif // KEYVALUELIST_H

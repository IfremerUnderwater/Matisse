#ifndef MATISSE_KEY_VALUE_LIST_H_
#define MATISSE_KEY_VALUE_LIST_H_

#include <QPair>
#include <QList>
#include <QString>
#include <QStringList>

///
/// Equivalent to a map class
///


namespace matisse {

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

} // namespace matisse

#endif // MATISSE_KEY_VALUE_LIST_H_

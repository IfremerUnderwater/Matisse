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
    KeyValueList(KeyValueList const &_other);
    bool append(QString _key, QString _value = "");
    void set(QString _key, QString _value);
    bool insert(QString _key, QString _value, int _before = -1);
    int getSize();
    void clear();
    QStringList getKeys();
    QStringList getValues();
    QString getValue(QString _key);

private:
    QStringList m_keys;
    QStringList m_values;
};

} // namespace matisse

#endif // MATISSE_KEY_VALUE_LIST_H_

#include "key_value_list.h"

namespace matisse {

KeyValueList::KeyValueList()
{
}

KeyValueList::KeyValueList(const KeyValueList &_other)
{
    m_keys = _other.m_keys;
    m_values = _other.m_values;
}

bool KeyValueList::append(QString _key, QString _value)
{
    if (m_keys.contains(_key)) {
        return false;
    }
    m_keys.append(_key);
    m_values.append(_value);

    return true;
}

void KeyValueList::set(QString _key, QString _value)
{
    int index = m_keys.indexOf(_key);
    if (index < 0) {
        m_keys.append(_key);
        m_values.append(_value);
    } else {
        m_keys.replace(index, _key);
        m_values.replace(index, _value);
    }
}

bool KeyValueList::insert(QString _key, QString _value, int _before)
{
    int size = m_keys.size();
    if (_before < 0){
        _before = size;
    }
    _before = qBound(0, _before, size);

    if (m_keys.contains(_key)) {
        return false;
    }

    m_keys.insert(_before, _key);
    m_values.insert(_before, _value);

    return true;
}

int KeyValueList::getSize()
{
    return m_keys.size();
}

void KeyValueList::clear()
{
    m_keys.clear();
    m_values.clear();
}

QStringList KeyValueList::getKeys()
{
    return m_keys;
}

QStringList KeyValueList::getValues()
{
    return m_values;
}

QString KeyValueList::getValue(QString _key)
{
    int index = m_keys.indexOf(_key);
    if (index < 0) {
        return "";
    }
    return m_values.at(index);
}

} // namespace matisse

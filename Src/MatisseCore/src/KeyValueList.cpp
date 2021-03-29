#include "KeyValueList.h"

using namespace MatisseTools;


KeyValueList::KeyValueList()
{
}

KeyValueList::KeyValueList(const KeyValueList &other)
{
    _keys = other._keys;
    _values = other._values;
}

bool KeyValueList::append(QString key, QString value)
{
    if (_keys.contains(key)) {
        return false;
    }
    _keys.append(key);
    _values.append(value);

    return true;
}

void KeyValueList::set(QString key, QString value)
{
    int index = _keys.indexOf(key);
    if (index < 0) {
        _keys.append(key);
        _values.append(value);
    } else {
        _keys.replace(index, key);
        _values.replace(index, value);
    }
}

bool KeyValueList::insert(QString key, QString value, int pos)
{
    int size = _keys.size();
    if (pos < 0){
        pos = size;
    }
    pos = qBound(0, pos, size);

    if (_keys.contains(key)) {
        return false;
    }

    _keys.insert(pos, key);
    _values.insert(pos, value);

    return true;
}

int KeyValueList::getSize()
{
    return _keys.size();
}

void KeyValueList::clear()
{
    _keys.clear();
    _values.clear();
}

QStringList KeyValueList::getKeys()
{
    return _keys;
}

QStringList KeyValueList::getValues()
{
    return _values;
}

QString KeyValueList::getValue(QString key)
{
    int index = _keys.indexOf(key);
    if (index < 0) {
        return "";
    }
    return _values.at(index);
}

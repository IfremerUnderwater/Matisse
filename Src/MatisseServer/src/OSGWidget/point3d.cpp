#include "point3d.h"

// from JSon to object
void Point3D::decode(QJsonObject & _obj)
{
    x = _obj.value("x").toDouble();
    y = _obj.value("y").toDouble();
    z = _obj.value("z").toDouble();
}

// from JSon to object
void Point3D::decode(QJsonObject & _obj, Point3D &_offset)
{
    x = _obj.value("x").toDouble() + _offset.x;
    y = _obj.value("y").toDouble() + _offset.y;
    z = _obj.value("z").toDouble() + _offset.z;
}


// encode to JSon
void Point3D::encode(QJsonObject & _obj)
{
    _obj.insert("x", QJsonValue(x));
    _obj.insert("y", QJsonValue(y));
    _obj.insert("z", QJsonValue(z));
}

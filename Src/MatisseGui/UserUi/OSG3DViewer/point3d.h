#ifndef MATISSE_POINT3D_H_
#define MATISSE_POINT3D_H_

#include <QJsonObject>

namespace matisse {

struct Point3D
{
    Point3D() :x(0), y(0), z(0) {}

    double x;
    double y;
    double z;

    // from JSon to object
    void decode(QJsonObject & _obj);
    void decode(QJsonObject & _obj, Point3D &_offset);

    // encode to JSon
    void encode(QJsonObject & _obj);
};

} // namespace matisse

#endif // MATISSE_POINT3D_H_


#ifndef RECONSTRUCTIONCONTEXT_H
#define RECONSTRUCTIONCONTEXT_H

#include <QMetaType>
#include <QVariant>

struct reconstructionContext {
  double lat_origin;
  double lon_origin;
  double alt_origin;
};

Q_DECLARE_METATYPE(reconstructionContext*)

#endif // RECONSTRUCTIONCONTEXT_H

#ifndef RECONSTRUCTIONCONTEXT_H
#define RECONSTRUCTIONCONTEXT_H

#include <QMetaType>
#include <QVariant>
#include <vector>

struct reconstructionContext {
  double lat_origin;
  double lon_origin;
  double alt_origin;
  std::vector<int> components_ids;
};

Q_DECLARE_METATYPE(reconstructionContext*)

#endif // RECONSTRUCTIONCONTEXT_H

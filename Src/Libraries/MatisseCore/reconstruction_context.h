#ifndef RECONSTRUCTIONCONTEXT_H
#define RECONSTRUCTIONCONTEXT_H

#include <QMetaType>
#include <QVariant>
#include <QString>
#include <vector>

enum class ReconFormat {
    openMVG,
    openMVS,
    MVE
};

struct reconstructionContext {
  double lat_origin;
  double lon_origin;
  double alt_origin;
  bool all_images_have_nav;
  std::vector<int> components_ids;
  ReconFormat current_format;
  QString out_file_suffix;
};

Q_DECLARE_METATYPE(reconstructionContext*)

#endif // RECONSTRUCTIONCONTEXT_H

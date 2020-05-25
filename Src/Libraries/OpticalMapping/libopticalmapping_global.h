#ifndef LIBOPTICALMAPPING_GLOBAL_H
#define LIBOPTICALMAPPING_GLOBAL_H

#include <QtCore/qglobal.h>

#if defined(LIBOPTICALMAPPING_LIBRARY)
#  define Q_DECL_EXPORT
#else
#  define Q_DECL_IMPORT
#endif

#endif // LIBOPTICALMAPPING_GLOBAL_H

#ifndef LIBMATISSECOMMON_GLOBAL_H
#define LIBMATISSECOMMON_GLOBAL_H

#include <QtCore/qglobal.h>

#if defined(LIBMATISSECOMMON_LIBRARY)
#  define Q_DECL_EXPORT
#else
#  define Q_DECL_IMPORT
#endif

#endif // LIBMATISSECOMMON_GLOBAL_H

#ifndef MATISSEMETATYPES
#define MATISSEMETATYPES

#include <QMetaType>
#include "NavPhotoInfoTcpListener.h"

Q_DECLARE_METATYPE(NavPhotoInfoMessage)


void RegisterCustomMetatypes(){
    qRegisterMetaType<NavPhotoInfoMessage>();
}

#endif // MATISSEMETATYPES

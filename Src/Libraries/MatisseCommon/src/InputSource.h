#ifndef INPUTSOURCE_H
#define INPUTSOURCE_H

#include "libmatissecommon_global.h"

namespace MatisseCommon {

///
/// \brief Classe de base des sources d'image.
///
class LIBMATISSECOMMONSHARED_EXPORT InputSource
{
public:
    InputSource() {}
    virtual bool isValid()=0;
};

}

#endif // INPUTSOURCE_H

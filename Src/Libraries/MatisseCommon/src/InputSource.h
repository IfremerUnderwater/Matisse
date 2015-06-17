#ifndef INPUTSOURCE_H
#define INPUTSOURCE_H

namespace MatisseCommon {

///
/// \brief Classe de base des sources d'image.
///
class InputSource
{
public:
    InputSource() {}
    virtual bool isValid()=0;
};

}

#endif // INPUTSOURCE_H

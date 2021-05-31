#ifndef MATISSE_INPUT_SOURCE_H_
#define MATISSE_INPUT_SOURCE_H_



namespace matisse_image {

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

#endif // MATISSE_INPUT_SOURCE_H_

#ifndef IMAGE_H
#define IMAGE_H

#include <QSharedData>
#include <QExplicitlySharedDataPointer>

//#include <opencv/cv.h>
#include <opencv2/opencv.hpp>

using namespace cv;

namespace MatisseCommon {

///
/// \brief Classe de base de manipulation des Image dans Matisse.
/// L'image mémoire est stockée au format OpenCV Mat. Selon les implémentation l'image mémoire peut être chargée en lazy-loading
///
class Image
{
public:
    Image();

    ///
    /// \brief Copie une image.
    /// \param other Image origine
    ///  \param shallowCopy Si vrai, l'image Mat sous jacente est elle-même copiée
    ///
    Image(const Image &other, bool shallowCopy=true);

    ///
    /// \brief Construit une image à partir d'une image OpenCV Mat
    /// Fonction réservée aux ImageProvider
    /// \param id
    /// \param imageData
    ///
    Image(int id, Mat * imageData = 0);

    ///
    /// \brief Destructeur. Détruit l'image OpenCV sous-jacente
    ///
    virtual ~Image();


    int id() { return _id; }

    ///
    /// \brief Retourne l'image OpenCV associée
    /// \return
    ///
    virtual Mat * imageData() { return _imageData;}
    void releaseImageData();

    ///
    /// \brief Return width of _imageData
    /// \return width or -1 if not available
    ///
    virtual int width();
    ///
    /// \brief Return height of _imageData
    /// \return height or -1 if not available
    ///
    virtual int height();

    ///
    /// \brief Fonction de deboggage
    /// \return
    ///
    virtual QString dumpAttr();

protected:
    //QExplicitlySharedDataPointer<ImageData> _imageData;
    int _id;
    Mat * _imageData;
};



}

#endif // IMAGE_H

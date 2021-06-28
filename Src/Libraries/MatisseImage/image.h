#ifndef MATISSE_IMAGE_IMAGE_H_
#define MATISSE_IMAGE_IMAGE_H_



#include <QSharedData>
#include <QExplicitlySharedDataPointer>

#include <opencv2/opencv.hpp>

namespace matisse_image {

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
    /// \param _other Image origine
    ///  \param _shallow_copy Si vrai, l'image Mat sous jacente est elle-même copiée
    ///
    Image(const Image &_other, bool _shallow_copy=true);

    ///
    /// \brief Construit une image à partir d'une image OpenCV Mat
    /// Fonction réservée aux ImageProvider
    /// \param _id
    /// \param _image_data
    ///
    Image(int m_id, cv::Mat * _image_data = 0);

    ///
    /// \brief Destructeur. Détruit l'image OpenCV sous-jacente
    ///
    virtual ~Image();


    int id() { return m_id; }

    ///
    /// \brief Retourne l'image OpenCV associée
    /// \return
    ///
    virtual cv::Mat * imageData() { return m_image_data;}
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
    int m_id;
    cv::Mat * m_image_data;
};



}

#endif // MATISSE_IMAGE_IMAGE_H_

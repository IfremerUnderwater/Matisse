#include "file_image.h"
#include <QDebug>

using namespace cv;

namespace matisse_image {

FileImage::FileImage(PictureFileSet *_picture_file_set, QString _file_name, QString _source_name, QString _source_format, int _id, NavInfo _nav_info):
    NavImage(_id, NULL, _nav_info),
    m_file_name(_file_name),
    m_source_name(_source_name),
    m_source_format(_source_format),
    m_picture_file_set(_picture_file_set)
{
    m_im_reader = new QImageReader(m_picture_file_set->rootDirname() + "/" +m_file_name);
    m_scale_factor = 1.0;
}

FileImage::FileImage(const FileImage &_other):NavImage(_other),
    m_file_name(_other.m_file_name),
    m_source_name(_other.m_source_name),
    m_source_format(_other.m_source_format),
    m_picture_file_set(_other.m_picture_file_set)
{
    m_im_reader = new QImageReader(m_picture_file_set->rootDirname() + "/" +m_file_name);
}

FileImage::~FileImage()
{
    delete m_im_reader;
}

QString FileImage::dumpAttr()
{
    QString ret(NavImage::dumpAttr());
    ret += "fileName = " + m_file_name + "\n";
    ret += "sourceName = " + m_source_name + "\n";
    ret += "sourceFormat = " + m_source_format + "\n";

    return ret;

}

double FileImage::getScaleFactor() const
{
    return m_scale_factor;
}

void FileImage::setScaleFactor(double _scale_factor)
{
    m_scale_factor = _scale_factor;
}

Mat *FileImage::imageData() {

    if ( m_image_data == 0) {
        // chargement de l'image
        // normalement elle existe car vérifiée dans le provider...
        if (!m_picture_file_set == 0) {
            std::string file_path =  QString(m_picture_file_set -> rootDirname() + "/" +m_file_name).toStdString();

            if (m_scale_factor < 1.0){
                m_image_data = new Mat();
                Mat fullSizeImg = imread(file_path, cv::IMREAD_COLOR | cv::IMREAD_IGNORE_ORIENTATION);
                resize(fullSizeImg, *m_image_data, cv::Size(0, 0), m_scale_factor, m_scale_factor);
            }else{
                m_image_data = new Mat(imread(file_path, cv::IMREAD_COLOR | cv::IMREAD_IGNORE_ORIENTATION));
            }
        }
    }

    return m_image_data;
}

int FileImage::width()
{
    if(m_image_data != 0){
        // If image is loaded send info from memory
        return m_image_data->cols;
    }else{
        // If not send info from reader (no image loading)
        if(m_im_reader){
            return m_scale_factor*m_im_reader->size().width();
        }else{
            qDebug() << "Image Size cannot be read";
            exit(1);
        }
    }
}

int FileImage::height()
{
    if(m_image_data != 0){
        // If image is loaded send info from memory
        return m_image_data->rows;
    }else{
        // If not send info from reader (no image loading)
        if(m_im_reader){
            return m_scale_factor*m_im_reader->size().height();
        }else{
            qDebug() << "Image Size cannot be read";
            exit(1);
        }
    }
}

} // namespace matisse_image

/*M///////////////////////////////////////////////////////////////////////////////////////
//
//  IMPORTANT: READ BEFORE DOWNLOADING, COPYING, INSTALLING OR USING.
//
//  By downloading, copying, installing or using the software you agree to this license.
//  If you do not agree to this license, do not download, install,
//  copy or use the software.
//
//
//                          License Agreement
//                For Open Source Computer Vision Library
//
// Copyright (C) 2000-2008, Intel Corporation, all rights reserved.
// Copyright (C) 2009, Willow Garage Inc., all rights reserved.
// Third party copyrights are property of their respective owners.
//
// Redistribution and use in source and binary forms, with or without modification,
// are permitted provided that the following conditions are met:
//
//   * Redistribution's of source code must retain the above copyright notice,
//     this list of conditions and the following disclaimer.
//
//   * Redistribution's in binary form must reproduce the above copyright notice,
//     this list of conditions and the following disclaimer in the documentation
//     and/or other materials provided with the distribution.
//
//   * The name of the copyright holders may not be used to endorse or promote products
//     derived from this software without specific prior written permission.
//
// This software is provided by the copyright holders and contributors "as is" and
// any express or implied warranties, including, but not limited to, the implied
// warranties of merchantability and fitness for a particular purpose are disclaimed.
// In no event shall the Intel Corporation or contributors be liable for any direct,
// indirect, incidental, special, exemplary, or consequential damages
// (including, but not limited to, procurement of substitute goods or services;
// loss of use, data, or profits; or business interruption) however caused
// and on any theory of liability, whether in contract, strict liability,
// or tort (including negligence or otherwise) arising in any way out of
// the use of this software, even if advised of the possibility of such damage.
//
//M*/

#include "file_img_precomp.hpp"
#include "file_img_exposure_compensate.h"
#include "opencv2/highgui/highgui.hpp"

#include <QFile>
#include <QDir>
#include <QTextStream>
#include <QDebug>

using namespace std;

namespace optical_mapping {

Ptr<FileImgExposureCompensator> FileImgExposureCompensator::createDefault(int type)
{
    if (type == NO)
        return new FileImgNoExposureCompensator();
    if (type == GAIN)
        return new FileImgGainCompensator();
    CV_Error(cv::Error::StsBadArg, "unsupported exposure compensation method");

    return (Ptr<FileImgExposureCompensator>)((FileImgExposureCompensator*)(NULL));
//    return NULL;
}
/*(const vector<Point> &corners, const std::vector<Mat> &images,
                           const std::vector<std::pair<Mat, uchar> > &masks)*/

void FileImgGainCompensator::feed(const QString & imagesPath_p, const QString & infoFilename_p)
{
#if ENABLE_LOG
    LOGLN("Exposure compensation...");
    int64 t = getTickCount();
#endif

    // Read info file to get images infos
    vector<Point> corners;
    vector<Size> imgsizes;
    vector<QString> imagesName;
    vector<QString> imagesMaskName;

    QFile infoFile(imagesPath_p + QDir::separator() + infoFilename_p);
    if(!infoFile.open(QIODevice::ReadOnly)) {
        qFatal("Invalid info file path provided ...");
    }

    QTextStream infoFileStream(&infoFile);

    while(!infoFileStream.atEnd()) {
        QString line = infoFileStream.readLine();
        QStringList infoFields = line.split(";");

        imagesName.push_back(infoFields[0]);
        imagesMaskName.push_back(infoFields[1]);
        corners.push_back(Point(infoFields[2].toInt(), infoFields[3].toInt()));
        imgsizes.push_back(Size(infoFields[4].toInt(),infoFields[5].toInt()));

    }

    infoFile.close();

    // Then compute
    CV_Assert(corners.size() == imgsizes.size());

    const int num_images = static_cast<int>(imgsizes.size());
    Mat_<int> N(num_images, num_images); N.setTo(0);
    Mat_<double> I(num_images, num_images); I.setTo(0);

    //Rect dst_roi = resultRoi(corners, images);
    Mat subimg1, subimg2;
    Mat image_i, image_j;
    Mat imagemask_i, imagemask_j;
    Mat_<uchar> submask1, submask2, intersect;

    for (int i = 0; i < num_images; ++i)
    {



        for (int j = i; j < num_images; ++j)
        {

            QString image_i_path = imagesPath_p + QDir::separator() + imagesName[i];
            QString imagemask_i_path = imagesPath_p + QDir::separator();
            imagemask_i_path += imagesMaskName[i];

            image_i = imread(image_i_path.toStdString().c_str(), cv::IMREAD_COLOR | cv::IMREAD_IGNORE_ORIENTATION);
            imagemask_i = imread(imagemask_i_path.toStdString().c_str(),cv::IMREAD_GRAYSCALE | cv::IMREAD_IGNORE_ORIENTATION);

            Rect roi;
            if (overlapRoi(corners[i], corners[j], imgsizes[i], imgsizes[j], roi))
            {


                QString image_j_path = imagesPath_p + QDir::separator() + imagesName[j];
                QString imagemask_j_path = imagesPath_p + QDir::separator();
                imagemask_j_path += imagesMaskName[j];

                image_j = imread(image_j_path.toStdString().c_str(), cv::IMREAD_COLOR | cv::IMREAD_IGNORE_ORIENTATION);
                imagemask_j = imread(imagemask_j_path.toStdString().c_str(),cv::IMREAD_GRAYSCALE | cv::IMREAD_IGNORE_ORIENTATION);

//qDebug() << "sub1 x, y = " << (roi.tl() - corners[i]).x << ", " << (roi.tl() - corners[i]).y << ", " << (roi.br() - corners[i]).x << ", " << (roi.br() - corners[i]).y;
//qDebug() << "sub2 x, y = " << (roi.tl() - corners[j]).x << ", " << (roi.tl() - corners[j]).y << ", " << (roi.br() - corners[j]).x << ", " << (roi.br() - corners[j]).y;

//qDebug() << "image1 size = " << image_i.cols << ", " << image_i.rows;
//qDebug() << "image2 size = " << image_j.cols << ", " << image_j.rows;

                subimg1 = image_i(Rect(roi.tl() - corners[i], roi.br() - corners[i]));
                subimg2 = image_j(Rect(roi.tl() - corners[j], roi.br() - corners[j]));

                submask1 = imagemask_i(Rect(roi.tl() - corners[i], roi.br() - corners[i]));
                submask2 = imagemask_j(Rect(roi.tl() - corners[j], roi.br() - corners[j]));
                intersect = (submask1 == 255) & (submask2 == 255);

                N(i, j) = N(j, i) = max(1, countNonZero(intersect));

                double Isum1 = 0, Isum2 = 0;
                for (int y = 0; y < roi.height; ++y)
                {
                    const Point3_<uchar>* r1 = subimg1.ptr<Point3_<uchar> >(y);
                    const Point3_<uchar>* r2 = subimg2.ptr<Point3_<uchar> >(y);
                    for (int x = 0; x < roi.width; ++x)
                    {
                        if (intersect(y, x))
                        {
                            Isum1 += sqrt(static_cast<double>(sqr(r1[x].x) + sqr(r1[x].y) + sqr(r1[x].z)));
                            Isum2 += sqrt(static_cast<double>(sqr(r2[x].x) + sqr(r2[x].y) + sqr(r2[x].z)));
                        }
                    }
                }
                I(i, j) = Isum1 / N(i, j);
                I(j, i) = Isum2 / N(i, j);
            }
        }
    }

    double alpha = 0.01;
    double beta = 100;

    Mat_<double> A(num_images, num_images); A.setTo(0);
    Mat_<double> b(num_images, 1); b.setTo(0);
    for (int i = 0; i < num_images; ++i)
    {
        for (int j = 0; j < num_images; ++j)
        {
            b(i, 0) += beta * N(i, j);
            A(i, i) += beta * N(i, j);
            if (j == i) continue;
            A(i, i) += 2 * alpha * I(i, j) * I(i, j) * N(i, j);
            A(i, j) -= 2 * alpha * I(i, j) * I(j, i) * N(i, j);
        }
    }

    solve(A, b, gains_);

#if ENABLE_LOG
    LOGLN("Exposure compensation, time: " << ((getTickCount() - t) / getTickFrequency()) << " sec");
#endif
}


void FileImgGainCompensator::apply(int index, Point /*corner*/, Mat &image, const Mat &/*mask*/)
{
    image *= gains_(index, 0);
}


vector<double> FileImgGainCompensator::gains() const
{
    vector<double> gains_vec(gains_.rows);
    for (int i = 0; i < gains_.rows; ++i)
        gains_vec[i] = gains_(i, 0);
    return gains_vec;
}

} // namespace optical_mapping

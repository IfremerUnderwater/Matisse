#include <QCoreApplication>
#include "imageprocessing.h"

int main(int argc, char *argv[])
{
    //QCoreApplication a(argc, argv);
    //return a.exec();

    cv::Mat in_img, out_img, empty_mask;

    in_img = cv::imread("/home/data/Test/20161012T113202.951596Z.jpg");

    histogramQuantileStretch(in_img, empty_mask, 0.0005,  out_img);

    cv::imwrite("/home/data/Test/20161012T113202.951596Z_corr.jpg",out_img);

    return 0;
}


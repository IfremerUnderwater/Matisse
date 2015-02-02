

#include "imageprocessor.h"



ImageProcessor *proc = 0;


void process(void* img) {

  IplImage* image = reinterpret_cast<IplImage*>(img);
  cvErode( image, image, 0, 2 );

}

ImageProcessor::~ImageProcessor() {

    cvReleaseImage( &img );
}


ImageProcessor::ImageProcessor(QString filename, bool display) {

    img = cvvLoadImage( qPrintable(filename) ); // load image

    if (display) {

        cvvNamedWindow( "Original Image", 1 );  // create a window
        cvvShowImage( "Original Image", img );  // display the image on window
    }
}

void ImageProcessor::display() {

    cvvNamedWindow( "Resulting Image", 1 );  // create a window
    cvvShowImage( "Resulting Image", img );  // display the image on window
}

void ImageProcessor::execute() {

    process(img);
}



#ifndef IMAGEPROCESSOR_H
#define IMAGEPROCESSOR_H

#include <QString>



#include "opencv/cv.h"      // include core library interface
#include "opencv/highgui.h" // include GUI library interface

class ImageProcessor {


public:

    ImageProcessor(QString filename, bool display=true);
    virtual ~ImageProcessor();

    void display();
    void execute();

private:
    IplImage* img; // Declare IPL/OpenCV image pointer
};

extern ImageProcessor *proc;

#endif // IMAGEPROCESSOR_H

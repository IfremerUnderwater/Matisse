#ifndef SEQUENCEPROCESSOR_H
#define SEQUENCEPROCESSOR_H

#include <QString>
#include <QStringList>

#include "opencv/cv.h"      // include core library interface
#include "opencv/highgui.h" // include GUI library interface


class IGraphBuilder;
class IMediaControl;
class IMediaEvent;

class SequenceProcessor {

private:
    IplImage* img; // Declare IPL/OpenCV image pointer
    IGraphBuilder *pGraph;
    IMediaControl *pMediaControl;
    IMediaEvent *pEvent;

public:

    SequenceProcessor(QString filename, bool display=true);

    QStringList enumFilters();

    virtual ~SequenceProcessor();
};


extern SequenceProcessor *procseq;

#endif // SEQUENCEPROCESSOR_H

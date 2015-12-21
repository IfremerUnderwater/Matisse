#ifndef VIDEOSOURCE_H
#define VIDEOSOURCE_H
#include <QObject>

#include "InputSource.h"


using namespace MatisseCommon;

class VideoSource : public QObject
{
    Q_OBJECT

signals:
    void imageReady();

public:
    VideoSource();

public slots:
    void slot_preview();

private:
};

#endif // VIDEOSOURCE_H

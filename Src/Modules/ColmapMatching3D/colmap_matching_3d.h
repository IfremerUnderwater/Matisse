#pragma once


#include "processor.h"


namespace matisse {


class ColmapMatching3D : public Processor
{
    Q_OBJECT
    Q_INTERFACES(matisse::Processor)

#if QT_VERSION >= QT_VERSION_CHECK(5, 0, 0)
    Q_PLUGIN_METADATA(IID "ColmapMatching3D")
#endif

public:
    ColmapMatching3D();
    ~ColmapMatching3D();
    virtual bool configure();
    virtual bool start();
    virtual bool stop();
    virtual void onFlush(quint32 _port);
    virtual void onNewImage(quint32 _port, matisse_image::Image &_image);


private:
    bool computeFeatures();
    bool computeMatches();

};

} // namespace matisse


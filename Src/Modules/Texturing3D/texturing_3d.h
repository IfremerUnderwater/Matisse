#ifndef InitMatchModule_H
#define InitMatchModule_H


#include "processor.h"

using namespace matisse;

/**
 * Module1
 * @brief  Exemple de module pour implementer un algorithme de traitement dans Matisse
 */
class Texturing3D : public Processor
{
    Q_OBJECT
    Q_INTERFACES(matisse::Processor)

#if QT_VERSION >= QT_VERSION_CHECK(5, 0, 0)
    Q_PLUGIN_METADATA(IID "Texturing3D")
#endif
public:
    Texturing3D();
    ~Texturing3D();
    virtual bool configure();
    virtual bool start();
    virtual bool stop();
    virtual void onFlush(quint32 port);
    virtual void onNewImage(quint32 port, Image &image);

private:
    QString m_source_dir;
    QString m_outdir;
    QString m_out_filename_prefix;

    bool generateCamFile(QString _sfm_data_file, QString _undist_img_path);
    void writeKml(QString model_path, QString model_prefix);
};

#endif // InitMatchModule_H

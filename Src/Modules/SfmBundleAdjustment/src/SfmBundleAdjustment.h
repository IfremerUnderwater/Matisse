#ifndef InitMatchModule_H
#define InitMatchModule_H

#include <QFileSystemWatcher>
#include <QDateTime>
#include "Processor.h"

using namespace MatisseCommon;

/**
 * Module1
 * @brief  Exemple de module pour implementer un algorithme de traitement dans Matisse
 */
class SfmBundleAdjustment : public Processor
{
    Q_OBJECT
    Q_INTERFACES(MatisseCommon::Processor)

#if QT_VERSION >= QT_VERSION_CHECK(5, 0, 0)
    Q_PLUGIN_METADATA(IID "SfmBundleAdjustment")
#endif

public:
    SfmBundleAdjustment();
    ~SfmBundleAdjustment();
    virtual bool configure();
    virtual bool start();
    virtual bool stop();
    virtual void onFlush(quint32 port);
    virtual void onNewImage(quint32 port, Image &image);

private:
    QDateTime m_start_time;
    QDateTime m_last_ply_time;
    QString m_root_dirname_str;
    QString m_out_dirname_str;
    QString m_out_complete_path_str;
    void checkForNewFiles();

};

#endif // InitMatchModule_H

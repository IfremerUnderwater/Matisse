#ifndef MATISSE_SFM_BUNDLE_ADJUSTMENT_H_
#define MATISSE_SFM_BUNDLE_ADJUSTMENT_H_

#include <QFileSystemWatcher>
#include <QDateTime>
#include "processor.h"

namespace matisse {

/**
 * Module1
 * @brief  Exemple de module pour implementer un algorithme de traitement dans Matisse
 */
class SfmBundleAdjustment : public Processor
{
    Q_OBJECT
    Q_INTERFACES(matisse::Processor)

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

    QString m_matches_path;
    QString m_splitted_matches_path;
    QString m_sfm_data_file;
    QString m_out_complete_path_str;
    QStringList m_matches_files_list;
    bool m_use_prior;
 
    void checkForNewFiles();
    bool splitMatchesFiles();
    bool incrementalSfm(QString _out_dir, QString _match_file);

};

} // namespace matisse

#endif // MATISSE_SFM_BUNDLE_ADJUSTMENT_H_

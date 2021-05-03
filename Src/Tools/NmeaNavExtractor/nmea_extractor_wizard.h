#ifndef NMEA_EXTRACTOR_WIZARD_H
#define NMEA_EXTRACTOR_WIZARD_H

#include <QWizard>
#include <QFileInfoList>
#include <QVector>
#include <QDateTime>
#include "nmea_files_reader.h"

namespace Ui {
class NmeaExtractorWizard;
}

class NmeaExtractorWizard : public QWizard
{
    Q_OBJECT

public:
    explicit NmeaExtractorWizard(QWidget *parent = nullptr);
    ~NmeaExtractorWizard();

public slots:
    void sl_selectNmeaPath();
    void sl_selectNavOutputFile();
    void sl_pageChanged(int _page_idx);
    void sl_finished(int _state);

private:

    void checkAndFillNmeaFiles(QString _dataPath );
    void generateNavFile(QFileInfo _video_file_info);

    Ui::NmeaExtractorWizard *ui;

    QString m_data_path;
    QString m_data_type;
    QFileInfoList m_found_video_files;
    QFileInfoList m_found_snap_files;
    QFileInfoList m_found_nmea_files;
    QVector<QDateTime> m_snap_files_times;

    NmeaFilesReader *m_nmea_file_reader;
};

#endif // NMEA_EXTRACTOR_WIZARD_H

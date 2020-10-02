#ifndef DATA_PREPROCESSING_WIZARD_H
#define DATA_PREPROCESSING_WIZARD_H

#include <QWizard>
#include "nav_file_reader.h"
#include "dim2_file_reader.h"

namespace Ui {
class DataPreprocessingWizard;
}

class DataPreprocessingWizard : public QWizard
{
    Q_OBJECT

public:
    explicit DataPreprocessingWizard(QWidget *parent = 0);
    ~DataPreprocessingWizard();

public slots:
    void sl_selectDataPath();
    void sl_selectDim2File();
    void sl_selectOutputPath();
    void sl_selectNavFile();
    void sl_pageChanged(int _page_idx);
    void sl_selectAllFiles();
    void sl_deselectAllFiles();
    void sl_finished(int _state);
    void sl_handleUseRtDim2();

private:

    void checkAndFillVideoFiles(QString _dataPath );
    void checkAndFillPhotoFiles(QString _dataPath );
    void video2Images();
    void preprocessImages();
    void preprocessImage(QString _image_path, QString _out_image_path);

    Ui::DataPreprocessingWizard *ui;

    QString m_data_path;
    QString m_data_type;
    QStringList m_found_files;

    NavFileReader *m_nav_file;
    Dim2FileReader* m_dim2_file;
};

#endif // DATA_PREPROCESSING_WIZARD_H

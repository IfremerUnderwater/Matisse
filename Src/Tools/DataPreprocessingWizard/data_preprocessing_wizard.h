#ifndef DATA_PREPROCESSING_WIZARD_H
#define DATA_PREPROCESSING_WIZARD_H

#include <QWizard>

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
    sl_selectDataPath();
    sl_selectNavFile();

private:
    Ui::DataPreprocessingWizard *ui;
};

#endif // DATA_PREPROCESSING_WIZARD_H

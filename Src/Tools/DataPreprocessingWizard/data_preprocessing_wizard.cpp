#include "data_preprocessing_wizard.h"
#include "ui_data_preprocessing_wizard.h"
#include <QMessageBox>
#include <QFileDialog>

DataPreprocessingWizard::DataPreprocessingWizard(QWidget *parent) :
    QWizard(parent),
    ui(new Ui::DataPreprocessingWizard)
{
    ui->setupUi(this);

    connect(ui->select_path, SIGNAL(released()), this, SLOT(sl_selectDataPath()));
    connect(ui->select_nav_file, SIGNAL(released()), this, SLOT(sl_selectNavFile()));

    //ui->data_selection_page->registerField("data_path_line*",ui->data_path_line);
}

DataPreprocessingWizard::~DataPreprocessingWizard()
{
    delete ui;
}

DataPreprocessingWizard::sl_selectDataPath()
{
    QString data_folder = QFileDialog::getExistingDirectory(this, tr("Data path"),
                                                            "./",
                                                            QFileDialog::ShowDirsOnly
                                                            | QFileDialog::DontResolveSymlinks);
    if (!data_folder.isEmpty())
    {
        ui->data_path_line->setText(data_folder);
    }
}

DataPreprocessingWizard::sl_selectNavFile()
{
    QString nav_file = QFileDialog::getOpenFileName(this,
                                                 tr("Open nav file"), "./", tr("Nav Files (*.txt)"));

    if (!nav_file.isEmpty())
    {
        ui->nav_file_line->setText(nav_file);
    }
}

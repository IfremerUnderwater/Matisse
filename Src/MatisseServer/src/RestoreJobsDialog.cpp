#include "RestoreJobsDialog.h"
#include "ui_RestoreJobsDialog.h"

using namespace MatisseServer;

RestoreJobsDialog::RestoreJobsDialog(QWidget *parent, QString assemblyName, QStringList archivedJobs, QStringList &jobsToRestore) :
    QDialog(parent),
    _ui(new Ui::RestoreJobsDialog),
    _archivedJobs(archivedJobs)
{
    _ui->setupUi(this);

    QString labelText = _ui->_LA_jobsToRestore->text();
    QString newLabelText = labelText.arg(assemblyName);
    _ui->_LA_jobsToRestore->setText(newLabelText);
    _ui->_LA_jobsToRestore->setWordWrap(true);

    connect(_ui->_PB_save, SIGNAL(clicked()), this, SLOT(slot_close()));
    connect(_ui->_PB_cancel, SIGNAL(clicked()), this, SLOT(slot_close()));

    _jobsToRestore = &jobsToRestore;

    foreach (QString archivedJob, archivedJobs) {
        QListWidgetItem *item = new QListWidgetItem(archivedJob);
        item->setFlags(Qt::ItemIsEnabled | Qt::ItemIsUserCheckable);
        item->setCheckState(Qt::Unchecked);
        _ui->_LW_archivedJobs->addItem(item);
    }
}

RestoreJobsDialog::~RestoreJobsDialog()
{
    delete _ui;
}

void RestoreJobsDialog::slot_close()
{
    if (sender() == _ui->_PB_cancel) {
        reject();
    } else {
        for(int row = 0; row < _ui->_LW_archivedJobs->count(); row++)
        {
            QListWidgetItem *item = _ui->_LW_archivedJobs->item(row);
            if (item->checkState() == Qt::Checked) {
                _jobsToRestore->append(item->text());
            }
        }

        accept();
    }
}

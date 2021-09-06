#include "restore_jobs_dialog.h"
#include "ui_restore_jobs_dialog.h"

namespace matisse {

RestoreJobsDialog::RestoreJobsDialog(QWidget *_parent, QString _assembly_name, QStringList _archived_jobs, QStringList &_jobs_to_restore) :
    QDialog(_parent),
    m_ui(new Ui::RestoreJobsDialog),
    m_archived_jobs(_archived_jobs)
{
    m_ui->setupUi(this);

    QString label_text = m_ui->_LA_jobsToRestore->text();
    QString new_label_text = label_text.arg(_assembly_name);
    m_ui->_LA_jobsToRestore->setText(new_label_text);
    m_ui->_LA_jobsToRestore->setWordWrap(true);

    connect(m_ui->_PB_save, SIGNAL(clicked()), this, SLOT(sl_close()));
    connect(m_ui->_PB_cancel, SIGNAL(clicked()), this, SLOT(sl_close()));

    m_jobs_to_restore = &_jobs_to_restore;

    foreach (QString archived_job, _archived_jobs) {
        QListWidgetItem *item = new QListWidgetItem(archived_job);
        item->setFlags(Qt::ItemIsEnabled | Qt::ItemIsUserCheckable);
        item->setCheckState(Qt::Unchecked);
        m_ui->_LW_archivedJobs->addItem(item);
    }
}

RestoreJobsDialog::~RestoreJobsDialog()
{
    delete m_ui;
}

void RestoreJobsDialog::sl_close()
{
    if (sender() == m_ui->_PB_cancel) {
        reject();
    } else {
        for(int row = 0; row < m_ui->_LW_archivedJobs->count(); row++)
        {
            QListWidgetItem *item = m_ui->_LW_archivedJobs->item(row);
            if (item->checkState() == Qt::Checked) {
                m_jobs_to_restore->append(item->text());
            }
        }

        accept();
    }
}

} // namespace matisse

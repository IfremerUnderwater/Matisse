#ifndef MATISSE_RESTORE_JOBS_DIALOG_H_
#define MATISSE_RESTORE_JOBS_DIALOG_H_

#include <QDialog>
#include <QtDebug>

namespace Ui {
class RestoreJobsDialog;
}

namespace matisse {

class RestoreJobsDialog : public QDialog
{
    Q_OBJECT

public:
    RestoreJobsDialog(QWidget *_parent, QString _assembly_name, QStringList _archived_jobs, QStringList &_jobs_to_restore);
    ~RestoreJobsDialog();

protected slots:
    void sl_close();

private:
    Ui::RestoreJobsDialog *m_ui;
    QStringList m_archived_jobs;
    QStringList *m_jobs_to_restore;
};

} // namespace matisse

#endif // MATISSE_RESTORE_JOBS_DIALOG_H_

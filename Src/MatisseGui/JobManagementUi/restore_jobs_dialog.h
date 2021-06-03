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
    RestoreJobsDialog(QWidget *parent, QString assemblyName, QStringList archivedJobs, QStringList &jobsToRestore);
    ~RestoreJobsDialog();

protected slots:
    void slot_close();

private:
    Ui::RestoreJobsDialog *_ui;
    QStringList _archivedJobs;
    QStringList *_jobsToRestore;
};

} // namespace matisse

#endif // MATISSE_RESTORE_JOBS_DIALOG_H_

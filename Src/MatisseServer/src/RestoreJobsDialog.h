#ifndef RESTOREJOBSDIALOG_H
#define RESTOREJOBSDIALOG_H

#include <QDialog>
#include <QtDebug>

namespace Ui {
class RestoreJobsDialog;
}

namespace MatisseServer {
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
}

#endif // RESTOREJOBSDIALOG_H

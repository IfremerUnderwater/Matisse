#ifndef DUPLICATEDIALOG_H
#define DUPLICATEDIALOG_H

#include <QDialog>
#include <QMessageBox>
#include <QDir>
#include <QFileInfo>

namespace Ui {
    class DuplicateDialog;
}

namespace MatisseServer {


class DuplicateDialog : public QDialog
{
    Q_OBJECT

public:
    //DuplicateDialog(QWidget *parent, QString originalName, QString &newName, QString basePath, QString archivePath, bool isAssembly = false);
    DuplicateDialog(QWidget *parent, QString originalName, QString &newName, bool isAssembly, QStringList existingElements, QStringList archivedJobs = QStringList());
    ~DuplicateDialog();

protected slots:
    void slot_close();

private:
    Ui::DuplicateDialog *_ui;

    QString _originalName;
    QString *_newName;
    bool _isAssembly;
    QStringList _existingElementNames;
    QStringList _archivedJobs;
};

}


#endif // DUPLICATEDIALOG_H

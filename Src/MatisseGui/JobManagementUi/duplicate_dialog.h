#ifndef MATISSE_DUPLICATE_DIALOG_H_
#define MATISSE_DUPLICATE_DIALOG_H_

#include <QDialog>
#include <QMessageBox>
#include <QDir>
#include <QFileInfo>

namespace Ui {
    class DuplicateDialog;
}

namespace matisse {


class DuplicateDialog : public QDialog
{
    Q_OBJECT

public:
    DuplicateDialog(QWidget *_parent, QString _original_name, QString &_new_name, bool _is_assembly, QStringList _existing_elements, QStringList _archived_jobs = QStringList());
    ~DuplicateDialog();

protected slots:
    void sl_close();

private:
    Ui::DuplicateDialog *m_ui;

    QString m_original_name;
    QString *m_new_name;
    bool m_is_assembly;
    QStringList m_existing_element_names;
    QStringList m_archived_jobs;
};

} // namespace matisse


#endif // MATISSE_DUPLICATE_DIALOG_H_

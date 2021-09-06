#ifndef JOBDIALOG_H
#define JOBDIALOG_H

#include <QFileInfo>
#include <QDialog>
#include <QDir>
#include <QRegExp>
#include <QRegExpValidator>
#include <QMessageBox>
#include <QFileDialog>

#include <QtDebug>

#include "key_value_list.h"
#include "parameters_common.h"
#include "matisse_icon_factory.h"
#include "iconized_button_wrapper.h"


namespace Ui {
class JobDialog;
}

namespace matisse {

class JobDialog : public QDialog
{
    Q_OBJECT

public:
    explicit JobDialog(QWidget *_parent,  MatisseIconFactory *_icon_factory, KeyValueList * _key_values, QString _jobs_path, QStringList _existing_job_names, QStringList _archived_job_names);
    ~JobDialog();

protected:
    void changeEvent(QEvent *_event); // overriding event handler for dynamic translation

private:
    Ui::JobDialog *m_ui;
    KeyValueList * m_key_values;
    QString m_jobs_path;
    QStringList m_existing_job_names;
    QStringList m_archived_job_names;

public slots:
    void sl_formatName(QString _text);
    void sl_close();
    void sl_selectDir();
    void sl_selectFile();
};

} // namespace matisse

#endif // JOBDIALOG_H

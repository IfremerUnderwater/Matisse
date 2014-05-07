#ifndef JOBDIALOG_H
#define JOBDIALOG_H

#include <QFileInfo>
#include <QDialog>
#include <QDir>
#include <QRegExp>
#include <QRegExpValidator>
#include <QMessageBox>

#include "KeyValueList.h"

using namespace MatisseTools;

namespace Ui {
class JobDialog;
}

namespace MatisseServer {
class JobDialog : public QDialog
{
    Q_OBJECT

public:
    explicit JobDialog(QWidget *parent = NULL, KeyValueList * keyValues = NULL, QString jobsPath = QString());
    ~JobDialog();

    static QString newJobName(QWidget * parent = NULL, KeyValueList * keyValues = NULL, QString jobsPath = QString());

private:
    Ui::JobDialog *_ui;
    KeyValueList * _keyValues;
    QString _jobsPath;

public slots:
    void slot_formatName(QString text);
    void slot_close();
};
}

#endif // JOBDIALOG_H

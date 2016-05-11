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

#include "KeyValueList.h"
#include "ToolsCommon.h"

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

protected:
    void changeEvent(QEvent *event); // overriding event handler for dynamic translation

private:
    Ui::JobDialog *_ui;
    KeyValueList * _keyValues;
    QString _jobsPath;
    bool _isRealTime;

public slots:
    void slot_formatName(QString text);
    void slot_close();
    void slot_selectDir();
    void slot_selectFile();
};
}

#endif // JOBDIALOG_H

#ifndef PARAMETERSDIALOG_H
#define PARAMETERSDIALOG_H

#include <QDialog>
#include <QFileInfo>
#include <QDir>
#include <QMessageBox>
#include <QRegExpValidator>
#include <QRegExp>
#include <QStringList>
#include <QDir>
#include <QDate>

#include "KeyValueList.h"

using namespace MatisseTools;

namespace Ui {
class ParametersDialog;
}

namespace MatisseServer {
class ParametersDialog : public QDialog
{
    Q_OBJECT

public:
    explicit ParametersDialog( QWidget *parent, QString path, KeyValueList fields = KeyValueList());
    ~ParametersDialog();

    QString getFilename();
    KeyValueList getFields();

private:
    Ui::ParametersDialog *_ui;
    QString _path;
    QString _filename;
    static QString _filenameExp;
    static QString _diacriticLetters;
    static QStringList _noDiacriticLetters;

    void init(KeyValueList fields);
    QString removeAccents(QString str);

protected slots:
    void slot_filename(QString name);
    void slot_save();
};
}

#endif // PARAMETERSDIALOG_H

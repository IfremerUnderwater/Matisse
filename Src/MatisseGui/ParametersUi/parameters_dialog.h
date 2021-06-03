#ifndef MATISSE_PARAMETERS_WIDGET_H_
#define MATISSE_PARAMETERS_WIDGET_H_

#include <QDialog>
#include <QFileInfo>
#include <QDir>
#include <QMessageBox>
#include <QRegExpValidator>
#include <QRegExp>
#include <QStringList>
#include <QDir>
#include <QDate>

#include "key_value_list.h"

namespace Ui {
class ParametersDialog;
}

namespace matisse {

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

} // namespace matisse

#endif // MATISSE_PARAMETERS_WIDGET_H_

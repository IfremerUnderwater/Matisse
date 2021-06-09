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
    explicit ParametersDialog( QWidget *_parent, QString _path, KeyValueList _fields = KeyValueList());
    ~ParametersDialog();

    QString getFilename();
    KeyValueList getFields();

private:
    Ui::ParametersDialog *m_ui;
    QString m_path;
    QString m_filename;
    static QString m_filename_exp;
    static QString m_diacritic_letters;
    static QStringList m_no_diacritic_letters;

    void init(KeyValueList _fields);
    QString removeAccents(QString _str);

protected slots:
    void sl_filename(QString _name);
    void sl_save();
};

} // namespace matisse

#endif // MATISSE_PARAMETERS_WIDGET_H_

#ifndef MATISSE_ASSEMBLY_DIALOG_H_
#define MATISSE_ASSEMBLY_DIALOG_H_

#include <QDialog>
#include <QDateTime>

#include "key_value_list.h"


namespace Ui {
class AssemblyDialog;
}

namespace matisse {

class AssemblyDialog : public QDialog
{
    Q_OBJECT
    
public:
    explicit AssemblyDialog(QWidget *parent, QString & name, KeyValueList & keyValue, bool isNewAssembly, bool isFirstTimeSave = false);
    ~AssemblyDialog();

protected:
    void changeEvent(QEvent *event); // overriding event handler for dynamic translation

private:
    Ui::AssemblyDialog *_ui;
    QString * _name;
    KeyValueList * _keyValue;

public slots:
    void slot_close();

signals:
    void signal_showWelcome();
};

} // namespace matisse

#endif // MATISSE_ASSEMBLY_DIALOG_H_

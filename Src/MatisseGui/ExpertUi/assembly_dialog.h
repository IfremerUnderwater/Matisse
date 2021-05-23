#ifndef ASSEMBLYDIALOG_H
#define ASSEMBLYDIALOG_H

#include <QDialog>
#include <QDateTime>

#include "key_value_list.h"

using namespace MatisseTools;

namespace Ui {
class AssemblyDialog;
}

namespace MatisseServer {
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
}
#endif // ASSEMBLYDIALOG_H

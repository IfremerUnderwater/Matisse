#ifndef ASSEMBLYDIALOG_H
#define ASSEMBLYDIALOG_H

#include <QDialog>
#include <QDateTime>

#include "KeyValueList.h"

using namespace MatisseTools;

namespace Ui {
class AssemblyDialog;
}

namespace MatisseServer {
class AssemblyDialog : public QDialog
{
    Q_OBJECT
    
public:
    explicit AssemblyDialog(QWidget *parent, QString & name, KeyValueList & keyValue);
    ~AssemblyDialog();


private:
    Ui::AssemblyDialog *_ui;
    QString * _name;
    KeyValueList * _keyValue;

public slots:
    void slot_close();
};
}
#endif // ASSEMBLYDIALOG_H

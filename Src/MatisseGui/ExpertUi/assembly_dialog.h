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
    explicit AssemblyDialog(QWidget *_parent, QString & _name, KeyValueList & _key_value, bool _is_new_assembly, bool _is_first_time_save = false);
    ~AssemblyDialog();

protected:
    void changeEvent(QEvent *_event); // overriding event handler for dynamic translation

private:
    Ui::AssemblyDialog *m_ui;
    QString * m_name;
    KeyValueList * m_key_value;

public slots:
    void sl_close();

signals:
    void si_showWelcome();
};

} // namespace matisse

#endif // MATISSE_ASSEMBLY_DIALOG_H_

#ifndef MATISSE_ABOUT_DIALOG_H_
#define MATISSE_ABOUT_DIALOG_H_

#include <QDialog>

#include "key_value_list.h"


namespace Ui {
    class AboutDialog;
}

namespace matisse {

class AboutDialog : public QDialog
{
    Q_OBJECT

public:
    AboutDialog(QWidget *_parent, KeyValueList _meta);

protected slots:
    void sl_close();

private:
    Ui::AboutDialog *m_ui;
};

} // namespace matisse

#endif // MATISSE_ABOUT_DIALOG_H_

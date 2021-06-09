#ifndef MATISSE_WELCOME_DIALOG_H_
#define MATISSE_WELCOME_DIALOG_H_

#include <QDialog>

#include "matisse_engine.h"
#include "matisse_icon_factory.h"
#include "iconized_button_wrapper.h"

namespace Ui {
class WelcomeDialog;
}

namespace matisse {

class WelcomeDialog : public QDialog
{
    Q_OBJECT

public:
    explicit WelcomeDialog(QWidget *_parent, MatisseIconFactory *_icon_factory, bool _is_programming_mode_enabled);
    ~WelcomeDialog();
    void enableProgrammingMode(bool _is_programming_mode_enabled);
    void dpiScale();

signals:
    void si_launchApplication(eApplicationMode _mode);

private slots:
    void on_progModeLauncherButton_clicked();
    void on_configModeLauncherButton_clicked();
    void on_deferredTimeModeLauncherButton_clicked();

protected:
    void changeEvent(QEvent *_event); // overriding event handler for dynamic translation

private:
    void fillOverlayLabel();
    Ui::WelcomeDialog *m_ui;
    QLabel* m_matisse_welcome_label;
};

} // namespace matisse

#endif // MATISSE_WELCOME_DIALOG_H_

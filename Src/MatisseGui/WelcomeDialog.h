#ifndef MATISSE_WELCOME_DIALOG_H_
#define MATISSE_WELCOME_DIALOG_H_

#include <QDialog>

#include "MatisseEngine.h"
#include "MatisseIconFactory.h"
#include "IconizedButtonWrapper.h"

namespace Ui {
class WelcomeDialog;
}

using namespace MatisseTools;

namespace MatisseServer {

class WelcomeDialog : public QDialog
{
    Q_OBJECT

public:
    explicit WelcomeDialog(QWidget *parent, MatisseIconFactory *iconFactory, bool isProgrammingModeEnabled);
    ~WelcomeDialog();
    void enableProgrammingMode(bool isProgrammingModeEnabled);
    void dpiScale();

signals:
    void signal_launchApplication(ApplicationMode mode);

private slots:
    void on_progModeLauncherButton_clicked();
    void on_configModeLauncherButton_clicked();
    void on_deferredTimeModeLauncherButton_clicked();

protected:
    void changeEvent(QEvent *event); // overriding event handler for dynamic translation

private:
    void fillOverlayLabel();
    Ui::WelcomeDialog *ui;
    QLabel* _matisseWelcomeLabel;
};
}

#endif // MATISSE_WELCOME_DIALOG_H_

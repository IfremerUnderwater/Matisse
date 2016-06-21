#ifndef WELCOMEDIALOG_H
#define WELCOMEDIALOG_H

#include <QDialog>

#include "Server.h"
#include "MatisseIconFactory.h"
#include "IconizedButtonWrapper.h"

namespace Ui {
class WelcomeDialog;
}

namespace MatisseServer {

class WelcomeDialog : public QDialog
{
    Q_OBJECT

public:
    explicit WelcomeDialog(QWidget *parent, MatisseIconFactory *iconFactory, bool isProgrammingModeEnabled);
    ~WelcomeDialog();
    void enableProgrammingMode(bool isProgrammingModeEnabled);

signals:
    void signal_launchApplication(ApplicationMode mode);

private slots:
    void on_progModeLauncherButton_clicked();
    void on_realTimeModeLauncherButton_clicked();
    void on_configModeLauncherButton_clicked();
    void on_deferredTimeModeLauncherButton_clicked();

protected:
    void changeEvent(QEvent *event); // overriding event handler for dynamic translation

private:
    void createOverlayLabel();
    Ui::WelcomeDialog *ui;
};
}

#endif // WELCOMEDIALOG_H

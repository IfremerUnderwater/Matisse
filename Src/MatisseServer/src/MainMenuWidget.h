#ifndef MAINMENUWIDGET_H
#define MAINMENUWIDGET_H

#include <QFrame>

namespace Ui {
class MainMenuWidget;
}

class MainMenuWidget : public QFrame
{
    Q_OBJECT

public:
    explicit MainMenuWidget(QWidget *parent = 0);
    ~MainMenuWidget();

protected:
    void changeEvent(QEvent *event); // overriding event handler for dynamic translation

private:
    Ui::MainMenuWidget *ui;
};

#endif // MAINMENUWIDGET_H

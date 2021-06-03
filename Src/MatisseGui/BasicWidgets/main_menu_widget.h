#ifndef MATISSE_MAIN_MENU_WIDGET_H_
#define MATISSE_MAIN_MENU_WIDGET_H_

#include <QFrame>

namespace Ui {
class MainMenuWidget;
}

namespace matisse {

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

} // namespace matisse

#endif // MATISSE_MAIN_MENU_WIDGET_H_

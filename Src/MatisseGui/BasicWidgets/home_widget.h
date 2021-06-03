#ifndef MATISSE_HOME_WIDGET_H_
#define MATISSE_HOME_WIDGET_H_

#include <QFrame>

namespace Ui {
class HomeWidget;
}

namespace matisse {

class HomeWidget : public QFrame
{
    Q_OBJECT

public:
    explicit HomeWidget(QWidget *parent = 0);
    ~HomeWidget();

signals:
    void signal_goHome();

public slots:
    void slot_goHome();

private:
    Ui::HomeWidget *ui;
};

} // namespace matisse

#endif // MATISSE_HOME_WIDGET_H_

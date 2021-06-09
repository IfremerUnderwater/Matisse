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
    explicit HomeWidget(QWidget *_parent = 0);
    ~HomeWidget();

signals:
    void si_goHome();

public slots:
    void sl_goHome();

private:
    Ui::HomeWidget *m_ui;
};

} // namespace matisse

#endif // MATISSE_HOME_WIDGET_H_

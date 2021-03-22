#ifndef HOMEWIDGET_H
#define HOMEWIDGET_H

#include <QFrame>

namespace Ui {
class HomeWidget;
}

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

#endif // HOMEWIDGET_H

#ifndef ONGOINGPROCESSWIDGET_H
#define ONGOINGPROCESSWIDGET_H

#include <QFrame>

namespace Ui {
class OngoingProcessWidget;
}

class OngoingProcessWidget : public QFrame
{
    Q_OBJECT

public:
    explicit OngoingProcessWidget(QWidget *parent = 0);
    ~OngoingProcessWidget();

protected:
    void changeEvent(QEvent *event); // overriding event handler for dynamic translation

private:
    Ui::OngoingProcessWidget *ui;
};

#endif // ONGOINGPROCESSWIDGET_H

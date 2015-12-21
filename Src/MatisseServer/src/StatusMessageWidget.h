#ifndef STATUSMESSAGEWIDGET_H
#define STATUSMESSAGEWIDGET_H

#include <QWidget>

namespace Ui {
class StatusMessageWidget;
}

class StatusMessageWidget : public QWidget
{
    Q_OBJECT

public:
    explicit StatusMessageWidget(QWidget *parent = 0);
    ~StatusMessageWidget();
    void addMessage(QString message, QIcon icon);

protected:
    void changeEvent(QEvent *event); // overriding event handler for dynamic translation

private:
    Ui::StatusMessageWidget *_ui;

};

#endif // STATUSMESSAGEWIDGET_H

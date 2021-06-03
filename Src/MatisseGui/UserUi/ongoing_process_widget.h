#ifndef MATISSE_ONGOING_PROCESS_WIDGET_H_
#define MATISSE_ONGOING_PROCESS_WIDGET_H_

#include <QFrame>

namespace Ui {
class OngoingProcessWidget;
}

namespace matisse {

class OngoingProcessWidget : public QFrame
{
    Q_OBJECT

public:
    explicit OngoingProcessWidget(QWidget *parent = 0);
    void dpiScale();
    ~OngoingProcessWidget();

protected:
    void changeEvent(QEvent *event); // overriding event handler for dynamic translation

private:
    Ui::OngoingProcessWidget *ui;
};

} // namespace matisse

#endif // MATISSE_ONGOING_PROCESS_WIDGET_H_

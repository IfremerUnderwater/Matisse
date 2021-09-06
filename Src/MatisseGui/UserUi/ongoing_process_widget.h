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
    explicit OngoingProcessWidget(QWidget *_parent = 0);
    void dpiScale();
    ~OngoingProcessWidget();

protected:
    void changeEvent(QEvent *_event); // overriding event handler for dynamic translation

private:
    Ui::OngoingProcessWidget *m_ui;
};

} // namespace matisse

#endif // MATISSE_ONGOING_PROCESS_WIDGET_H_

#ifndef MATISSE_VISU_MODE_WIDGET_H_
#define MATISSE_VISU_MODE_WIDGET_H_

#include <QFrame>

namespace Ui {
class VisuModeWidget;
}

namespace matisse {

class VisuModeWidget : public QFrame
{
    Q_OBJECT

public:
    explicit VisuModeWidget(QWidget *parent = 0);
    ~VisuModeWidget();

protected:
    void changeEvent(QEvent *event); // overriding event handler for dynamic translation

private:
    Ui::VisuModeWidget *ui;
};

} // namespace matisse

#endif // MATISSE_VISU_MODE_WIDGET_H_

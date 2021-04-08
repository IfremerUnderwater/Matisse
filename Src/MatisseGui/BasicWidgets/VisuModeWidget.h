#ifndef VISUMODEWIDGET_H
#define VISUMODEWIDGET_H

#include <QFrame>

namespace Ui {
class VisuModeWidget;
}

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

#endif // VISUMODEWIDGET_H

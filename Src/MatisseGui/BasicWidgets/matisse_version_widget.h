#ifndef MATISSE_MATISSE_VERSION_WIDGET_H_
#define MATISSE_MATISSE_VERSION_WIDGET_H_

#include <QFrame>

namespace Ui {
class MatisseVersionWidget;
}

namespace matisse {

class MatisseVersionWidget : public QFrame
{
    Q_OBJECT

public:
    explicit MatisseVersionWidget(QWidget *parent = 0);
    ~MatisseVersionWidget();

private:
    Ui::MatisseVersionWidget *ui;
};

} // namespace matisse

#endif // MATISSE_MATISSE_VERSION_WIDGET_H_

#ifndef MATISSEVERSIONWIDGET_H
#define MATISSEVERSIONWIDGET_H

#include <QFrame>

namespace Ui {
class MatisseVersionWidget;
}

class MatisseVersionWidget : public QFrame
{
    Q_OBJECT

public:
    explicit MatisseVersionWidget(QWidget *parent = 0);
    ~MatisseVersionWidget();

private:
    Ui::MatisseVersionWidget *ui;
};

#endif // MATISSEVERSIONWIDGET_H

#ifndef MAINCONTROLLBAR_H
#define MAINCONTROLLBAR_H

#include <QFrame>
#include <QMouseEvent>

namespace Ui {
class MainControllBar;
}

class MainControllBar : public QFrame
{
    Q_OBJECT

public:
    explicit MainControllBar(QWidget *parent = 0);
    ~MainControllBar();

    // surcharge des événements
    void mousePressEvent  (QMouseEvent *event);
    void mouseReleaseEvent(QMouseEvent *event);
    void mouseMoveEvent   (QMouseEvent *event);

    void setSwitchModeButtonEnable(bool enabled_p);

protected:
    void changeEvent(QEvent *event); // overriding event handler for dynamic translation

private:
    Ui::MainControllBar *ui;
    QPoint _initialPosition;


signals:
    void signal_moveWindow(const QPoint &newPos);
};

#endif // MAINCONTROLLBAR_H

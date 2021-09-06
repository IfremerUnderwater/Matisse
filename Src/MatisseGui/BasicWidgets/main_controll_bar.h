#ifndef MATISSE_MAIN_CONTROLL_BAR_H_
#define MATISSE_MAIN_CONTROLL_BAR_H_

#include <QFrame>
#include <QMouseEvent>

namespace Ui {
class MainControllBar;
}

namespace matisse {

class MainControllBar : public QFrame
{
    Q_OBJECT

public:
    explicit MainControllBar(QWidget *_parent = 0);
    ~MainControllBar();

    // surcharge des événements
    void mousePressEvent  (QMouseEvent *_event);
    void mouseReleaseEvent(QMouseEvent *_event);
    void mouseMoveEvent   (QMouseEvent *_event);

    void setSwitchModeButtonEnable(bool _enabled_p);

protected:
    void changeEvent(QEvent *_event); // overriding event handler for dynamic translation

private:
    Ui::MainControllBar *m_ui;
    QPoint m_initial_position;


signals:
    void si_moveWindow(const QPoint &_new_pos);
};

} // namespace matisse

#endif // MATISSE_MAIN_CONTROLL_BAR_H_

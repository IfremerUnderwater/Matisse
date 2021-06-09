#include "main_controll_bar.h"
#include "ui_main_controll_bar.h"

#include <QDebug>
#include <QToolButton>

namespace matisse {

MainControllBar::MainControllBar(QWidget *_parent) :
    QFrame(_parent),
    m_ui(new Ui::MainControllBar)
{
    m_ui->setupUi(this);
}

MainControllBar::~MainControllBar()
{
    delete m_ui;
}

void MainControllBar::mousePressEvent(QMouseEvent *_event)
{
    m_initial_position = _event->pos();
    setCursor(QCursor(Qt::SizeAllCursor));
}

void MainControllBar::mouseReleaseEvent(QMouseEvent *_event)
{
    Q_UNUSED(_event);

    setCursor(QCursor(Qt::ArrowCursor));
}

void MainControllBar::mouseMoveEvent(QMouseEvent *_event)
{
    QPoint current_pos = _event->globalPos();
    QPoint delta = current_pos - m_initial_position;
    //qDebug() << "Delta : " << currentPos - _initialPosition;
    emit si_moveWindow(delta);
}


void MainControllBar::setSwitchModeButtonEnable(bool _enabled_p)
{
    QToolButton* switch_mode_button = findChild<QToolButton*>(QString("_TBU_visuModeSwap"));
    switch_mode_button->setEnabled(_enabled_p);
}


void MainControllBar::changeEvent(QEvent *_event)
{
    if (_event->type() == QEvent::LanguageChange)
    {
        m_ui->retranslateUi(this);
    }
}

} // namespace matisse


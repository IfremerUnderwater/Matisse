#include "MainControllBar.h"
#include "ui_MainControllBar.h"

#include <QDebug>
#include <QToolButton>

MainControllBar::MainControllBar(QWidget *parent) :
    QFrame(parent),
    ui(new Ui::MainControllBar)
{
    ui->setupUi(this);
}

MainControllBar::~MainControllBar()
{
    delete ui;
}

void MainControllBar::mousePressEvent(QMouseEvent *event)
{
    _initialPosition = event->pos();
    setCursor(QCursor(Qt::SizeAllCursor));
}

void MainControllBar::mouseReleaseEvent(QMouseEvent *event)
{
    Q_UNUSED(event);

    setCursor(QCursor(Qt::ArrowCursor));
}

void MainControllBar::mouseMoveEvent(QMouseEvent *event)
{
    QPoint currentPos = event->globalPos();
    QPoint delta = currentPos - _initialPosition;
    //qDebug() << "Delta : " << currentPos - _initialPosition;
    emit signal_moveWindow(delta);
}


void MainControllBar::setSwitchModeButtonEnable(bool enabled_p)
{
    QToolButton* switchModeButton = findChild<QToolButton*>(QString("_TBU_visuModeSwap"));
    switchModeButton->setEnabled(enabled_p);
}


void MainControllBar::changeEvent(QEvent *event)
{
    if (event->type() == QEvent::LanguageChange)
    {
        ui->retranslateUi(this);
    }
}


#include "home_widget.h"
#include "ui_home_widget.h"

#include <QtDebug>

HomeWidget::HomeWidget(QWidget *parent) :
    QFrame(parent),
    ui(new Ui::HomeWidget)
{
    ui->setupUi(this);
    connect(ui->_TB_homeButton, SIGNAL(clicked()), this, SLOT(slot_goHome()));
}

HomeWidget::~HomeWidget()
{
    delete ui;
}

void HomeWidget::slot_goHome()
{
    emit signal_goHome();
}

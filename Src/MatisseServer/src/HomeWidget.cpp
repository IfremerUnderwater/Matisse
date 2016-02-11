#include "HomeWidget.h"
#include "ui_HomeWidget.h"

#include <QtDebug>

HomeWidget::HomeWidget(QWidget *parent) :
    QFrame(parent),
    ui(new Ui::HomeWidget)
{
    ui->setupUi(this);
    connect(ui->homeButton, SIGNAL(clicked()), this, SLOT(slot_goHome()));
}

HomeWidget::~HomeWidget()
{
    delete ui;
}

void HomeWidget::slot_goHome()
{
    emit signal_goHome();
}
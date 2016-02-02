#include "OngoingProcessWidget.h"
#include "ui_OngoingProcessWidget.h"

OngoingProcessWidget::OngoingProcessWidget(QWidget *parent) :
    QFrame(parent),
    ui(new Ui::OngoingProcessWidget)
{
    ui->setupUi(this);
}

OngoingProcessWidget::~OngoingProcessWidget()
{
    delete ui;
}

void OngoingProcessWidget::changeEvent(QEvent *event)
{
    if (event->type() == QEvent::LanguageChange)
    {
        ui->retranslateUi(this);
    }
}

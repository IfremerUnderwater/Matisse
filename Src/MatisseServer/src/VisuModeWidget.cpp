#include "VisuModeWidget.h"
#include "ui_VisuModeWidget.h"

VisuModeWidget::VisuModeWidget(QWidget *parent) :
    QFrame(parent),
    ui(new Ui::VisuModeWidget)
{
    ui->setupUi(this);
}

VisuModeWidget::~VisuModeWidget()
{
    delete ui;
}

void VisuModeWidget::changeEvent(QEvent *event)
{
    if (event->type() == QEvent::LanguageChange)
    {
        ui->retranslateUi(this);
    }
}

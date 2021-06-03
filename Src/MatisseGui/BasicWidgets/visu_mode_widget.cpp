#include "visu_mode_widget.h"
#include "ui_visu_mode_widget.h"

namespace matisse {

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

} // namespace matisse

#include "visu_mode_widget.h"
#include "ui_visu_mode_widget.h"

namespace matisse {

VisuModeWidget::VisuModeWidget(QWidget *_parent) :
    QFrame(_parent),
    m_ui(new Ui::VisuModeWidget)
{
    m_ui->setupUi(this);
}

VisuModeWidget::~VisuModeWidget()
{
    delete m_ui;
}

void VisuModeWidget::changeEvent(QEvent *_event)
{
    if (_event->type() == QEvent::LanguageChange)
    {
        m_ui->retranslateUi(this);
    }
}

} // namespace matisse

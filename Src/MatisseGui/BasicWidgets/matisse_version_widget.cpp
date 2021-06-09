#include "matisse_version_widget.h"
#include "ui_matisse_version_widget.h"

namespace matisse {

MatisseVersionWidget::MatisseVersionWidget(QWidget *_parent) :
    QFrame(_parent),
    m_ui(new Ui::MatisseVersionWidget)
{
    m_ui->setupUi(this);
}

MatisseVersionWidget::~MatisseVersionWidget()
{
    delete m_ui;
}

} // namespace matisse

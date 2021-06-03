#include "matisse_version_widget.h"
#include "ui_matisse_version_widget.h"

namespace matisse {

MatisseVersionWidget::MatisseVersionWidget(QWidget *parent) :
    QFrame(parent),
    ui(new Ui::MatisseVersionWidget)
{
    ui->setupUi(this);
}

MatisseVersionWidget::~MatisseVersionWidget()
{
    delete ui;
}

} // namespace matisse

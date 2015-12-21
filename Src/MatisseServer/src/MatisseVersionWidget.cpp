#include "MatisseVersionWidget.h"
#include "ui_MatisseVersionWidget.h"

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

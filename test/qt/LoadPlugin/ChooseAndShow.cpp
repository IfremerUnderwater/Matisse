#include "ChooseAndShow.h"
#include "ui_ChooseAndShow.h"

ChooseAndShow::ChooseAndShow(QString buttonText, QWidget *parent) :
    QWidget(parent),
    ui(new Ui::ChooseAndShow)
{
    ui->setupUi(this);
    ui->_PB_choose->setText(buttonText);
    connect(ui->_PB_choose, SIGNAL(clicked()), this, SLOT(slot_clicked()));
}

ChooseAndShow::~ChooseAndShow()
{
    delete ui;
}

void ChooseAndShow::setText(QString text)
{
    ui->_LE_show->setText(text);
}

void ChooseAndShow::slot_clicked() {
    emit signal_clicked();
}

QString ChooseAndShow::text()
{
    return ui->_LE_show->text();
}

QString ChooseAndShow::buttonText()
{
    return ui->_PB_choose->text();
}

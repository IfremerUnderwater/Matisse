#include "TestAppliQt.h"
#include "ui_TestAppliQt.h"

TestAppliQt::TestAppliQt(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::TestAppliQt)
{
    ui->setupUi(this);
}

TestAppliQt::~TestAppliQt()
{
    delete ui;
}

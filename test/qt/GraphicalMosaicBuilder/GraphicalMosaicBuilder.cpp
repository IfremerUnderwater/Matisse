#include "GraphicalMosaicBuilder.h"
#include "ui_GraphicalMosaicBuilder.h"

GraphicalMosaicBuilder::GraphicalMosaicBuilder(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::GraphicalMosaicBuilder)
{
    ui->setupUi(this);
}

GraphicalMosaicBuilder::~GraphicalMosaicBuilder()
{
    delete ui;
}

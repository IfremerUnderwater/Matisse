#include "main_menu_widget.h"
#include "ui_main_menu_widget.h"
#include "graphical_charter.h"

#include <QVBoxLayout>
#include <QMenuBar>
#include <QMenu>
#include <QVBoxLayout>

using namespace matisse;

MainMenuWidget::MainMenuWidget(QWidget *parent) :
    QFrame(parent),
    ui(new Ui::MainMenuWidget)
{
    ui->setupUi(this);

    QVBoxLayout *boxLayout = new QVBoxLayout(this); // Main layout of widget
    boxLayout->setAlignment(Qt::AlignVCenter);
    boxLayout->setContentsMargins(0,0,0,0);
    setLayout(boxLayout);

    QMenuBar* menuBar = new QMenuBar(this);
    menuBar->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Fixed);
    menuBar->setObjectName("_MBA_mainMenuBar");
    this->layout()->addWidget(menuBar);

}

MainMenuWidget::~MainMenuWidget()
{
    delete ui;
}

void MainMenuWidget::changeEvent(QEvent *event)
{
    if (event->type() == QEvent::LanguageChange)
    {
        ui->retranslateUi(this);
    }
}

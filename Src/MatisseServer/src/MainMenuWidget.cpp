#include "MainMenuWidget.h"
#include "ui_MainMenuWidget.h"

#include <QVBoxLayout>
#include <QMenuBar>
#include <QMenu>
#include <QVBoxLayout>

MainMenuWidget::MainMenuWidget(QWidget *parent) :
    QFrame(parent),
    ui(new Ui::MainMenuWidget)
{
    ui->setupUi(this);

    QVBoxLayout *boxLayout = new QVBoxLayout(this); // Main layout of widget
    boxLayout->setAlignment(Qt::AlignCenter);
    boxLayout->setContentsMargins(0,0,0,0);

    QMenuBar* menuBar = new QMenuBar();

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

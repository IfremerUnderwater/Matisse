#include <QtDebug>

#include "WelcomeDialog.h"
#include "ui_WelcomeDialog.h"

#include <QLabel>

using namespace MatisseServer;

WelcomeDialog::WelcomeDialog(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::WelcomeDialog)
{
    ui->setupUi(this);

    createOverlayLabel();

    connect(this, SIGNAL(signal_launchApplication(ApplicationMode)), parentWidget(), SLOT(slot_showApplicationMode(ApplicationMode)));
    connect(parentWidget(), SIGNAL(signal_showWelcome()), this, SLOT(slot_showWelcome()));
    connect(ui->_TBU_welcomeCloseButton, SIGNAL(clicked()), this, SLOT(close()));
}

WelcomeDialog::~WelcomeDialog()
{
    delete ui;
}

void WelcomeDialog::createOverlayLabel()
{
    QLabel* matisseWelcomeLabel = new QLabel(this->ui->gridLayoutWidget);
    matisseWelcomeLabel->setObjectName("_LA_matisseWelcomeLabel");
    matisseWelcomeLabel->setText("MATISSE");
    matisseWelcomeLabel->resize(150, 60);
    matisseWelcomeLabel->move(95,140);
    matisseWelcomeLabel->setAlignment(Qt::AlignCenter);
    matisseWelcomeLabel->raise();
}

void WelcomeDialog::on_progModeLauncherButton_clicked()
{
    hide();
    emit signal_launchApplication(PROGRAMMING);
}

void WelcomeDialog::on_realTimeModeLauncherButton_clicked()
{
    hide();
    emit signal_launchApplication(REAL_TIME);
}

void WelcomeDialog::on_configModeLauncherButton_clicked()
{
    hide();
    emit signal_launchApplication(APP_CONFIG);
}

void WelcomeDialog::on_deferredTimeModeLauncherButton_clicked()
{
    hide();
    emit signal_launchApplication(DEFERRED_TIME);
}

void WelcomeDialog::slot_showWelcome()
{
    show();
}

void WelcomeDialog::changeEvent(QEvent *event)
{
    if (event->type() == QEvent::LanguageChange)
    {
        ui->retranslateUi(this);
    }
}

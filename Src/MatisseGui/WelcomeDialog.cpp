#include <QtDebug>

#include "WelcomeDialog.h"
#include "ui_WelcomeDialog.h"
#include "graphical_charter.h"

#include <QLabel>

using namespace MatisseServer;

WelcomeDialog::WelcomeDialog(QWidget *parent, MatisseIconFactory *iconFactory, bool isProgrammingModeEnabled) :
    QDialog(parent),
    ui(new Ui::WelcomeDialog)
{
    ui->setupUi(this);

    _matisseWelcomeLabel = new QLabel(this);
    dpiScale();

    ui->progModeLauncherButton->setEnabled(isProgrammingModeEnabled);

    connect(this, SIGNAL(signal_launchApplication(ApplicationMode)), parentWidget(), SLOT(slot_showApplicationMode(ApplicationMode)));
    connect(ui->_TBU_welcomeCloseButton, SIGNAL(clicked()), this, SLOT(close()));

    IconizedButtonWrapper *closeButtonWrapper = new IconizedButtonWrapper(ui->_TBU_welcomeCloseButton);
    iconFactory->attachIcon(closeButtonWrapper, "lnf/icons/fermer.svg", false, false);
}

WelcomeDialog::~WelcomeDialog()
{
    delete ui;
    delete _matisseWelcomeLabel;
}

void WelcomeDialog::enableProgrammingMode(bool isProgrammingModeEnabled)
{
    ui->progModeLauncherButton->setEnabled(isProgrammingModeEnabled);
}

void WelcomeDialog::dpiScale()
{
    GraphicalCharter &graph_charter = GraphicalCharter::instance();

    fillOverlayLabel();
    this->setFixedSize(graph_charter.dpiScaled(340),graph_charter.dpiScaled(340));
    ui->_TBU_welcomeCloseButton->setFixedSize(graph_charter.dpiScaled(28),graph_charter.dpiScaled(28));
    ui->_TBU_welcomeCloseButton->setIconSize(QSize(graph_charter.dpiScaled(20),graph_charter.dpiScaled(20)));
    ui->_TBU_welcomeEmptyButton->setFixedSize(graph_charter.dpiScaled(28),graph_charter.dpiScaled(28));

}

void WelcomeDialog::fillOverlayLabel()
{
    GraphicalCharter &graph_charter = GraphicalCharter::instance();

    _matisseWelcomeLabel->setObjectName("_LA_matisseWelcomeLabel");
    _matisseWelcomeLabel->setText("MATISSE");
    _matisseWelcomeLabel->resize(graph_charter.dpiScaled(150), graph_charter.dpiScaled(60));
    _matisseWelcomeLabel->move(graph_charter.dpiScaled(95),graph_charter.dpiScaled(140));
    _matisseWelcomeLabel->setAlignment(Qt::AlignCenter);
    _matisseWelcomeLabel->raise();
}

void WelcomeDialog::on_progModeLauncherButton_clicked()
{
    hide();
    emit signal_launchApplication(PROGRAMMING);
}

void WelcomeDialog::on_configModeLauncherButton_clicked()
{
    hide();
    emit signal_launchApplication(APP_CONFIG);
}

void WelcomeDialog::on_deferredTimeModeLauncherButton_clicked()
{
    hide();
    emit signal_launchApplication(POST_PROCESSING);
}

void WelcomeDialog::changeEvent(QEvent *event)
{
    if (event->type() == QEvent::LanguageChange)
    {
        ui->retranslateUi(this);
    }
}

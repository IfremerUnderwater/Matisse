#include <QtDebug>

#include "welcome_dialog.h"
#include "ui_welcome_dialog.h"
#include "graphical_charter.h"

#include <QLabel>

namespace matisse {

WelcomeDialog::WelcomeDialog(QWidget *_parent, MatisseIconFactory *_icon_factory, bool _is_programming_mode_enabled) :
    QDialog(_parent),
    m_ui(new Ui::WelcomeDialog)
{
    m_ui->setupUi(this);

    m_matisse_welcome_label = new QLabel(this);
    dpiScale();

    m_ui->progModeLauncherButton->setEnabled(_is_programming_mode_enabled);

    connect(this, SIGNAL(si_launchApplication(eApplicationMode)), parentWidget(), SLOT(sl_showApplicationMode(eApplicationMode)));
    connect(m_ui->_TBU_welcomeCloseButton, SIGNAL(clicked()), this, SLOT(close()));

    IconizedButtonWrapper *close_button_wrapper = new IconizedButtonWrapper(m_ui->_TBU_welcomeCloseButton);
    _icon_factory->attachIcon(close_button_wrapper, "lnf/icons/fermer.svg", false, false);
}

WelcomeDialog::~WelcomeDialog()
{
    delete m_ui;
    delete m_matisse_welcome_label;
}

void WelcomeDialog::enableProgrammingMode(bool _is_programming_mode_enabled)
{
    m_ui->progModeLauncherButton->setEnabled(_is_programming_mode_enabled);
}

void WelcomeDialog::dpiScale()
{
    GraphicalCharter &graph_charter = GraphicalCharter::instance();

    fillOverlayLabel();
    this->setFixedSize(graph_charter.dpiScaled(340),graph_charter.dpiScaled(340));
    m_ui->_TBU_welcomeCloseButton->setFixedSize(graph_charter.dpiScaled(28),graph_charter.dpiScaled(28));
    m_ui->_TBU_welcomeCloseButton->setIconSize(QSize(graph_charter.dpiScaled(20),graph_charter.dpiScaled(20)));
    m_ui->_TBU_welcomeEmptyButton->setFixedSize(graph_charter.dpiScaled(28),graph_charter.dpiScaled(28));

}

void WelcomeDialog::fillOverlayLabel()
{
    GraphicalCharter &graph_charter = GraphicalCharter::instance();

    m_matisse_welcome_label->setObjectName("_LA_matisseWelcomeLabel");
    m_matisse_welcome_label->setText("MATISSE");
    m_matisse_welcome_label->resize(graph_charter.dpiScaled(150), graph_charter.dpiScaled(60));
    m_matisse_welcome_label->move(graph_charter.dpiScaled(95),graph_charter.dpiScaled(140));
    m_matisse_welcome_label->setAlignment(Qt::AlignCenter);
    m_matisse_welcome_label->raise();
}

void WelcomeDialog::on_progModeLauncherButton_clicked()
{
    hide();
    emit si_launchApplication(PROGRAMMING);
}

void WelcomeDialog::on_configModeLauncherButton_clicked()
{
    hide();
    emit si_launchApplication(APP_CONFIG);
}

void WelcomeDialog::on_deferredTimeModeLauncherButton_clicked()
{
    hide();
    emit si_launchApplication(POST_PROCESSING);
}

void WelcomeDialog::changeEvent(QEvent *_event)
{
    if (_event->type() == QEvent::LanguageChange)
    {
        m_ui->retranslateUi(this);
    }
}

} // namespace matisse

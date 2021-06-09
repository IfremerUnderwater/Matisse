#include "ongoing_process_widget.h"
#include "ui_ongoing_process_widget.h"
#include "graphical_charter.h"

namespace matisse {

OngoingProcessWidget::OngoingProcessWidget(QWidget *_parent) :
    QFrame(_parent),
    m_ui(new Ui::OngoingProcessWidget)
{
    m_ui->setupUi(this);
}

void OngoingProcessWidget::dpiScale()
{
    GraphicalCharter &graph_charter = GraphicalCharter::instance();
    int dpi_cb_height = graph_charter.dpiScaled(CONTROLLBAR_HEIGHT);

    int dpi_stop_wh = graph_charter.dpiScaled(CB_STOP_BUTTON_ICON);
    m_ui->_TBU_stopButton->setFixedSize(dpi_cb_height,dpi_cb_height);
    m_ui->_TBU_stopButton->setIconSize(QSize(dpi_stop_wh,dpi_stop_wh));

    m_ui->_WID_liveProcessWheel->setFixedSize(dpi_cb_height,dpi_cb_height);

    int butt_wh = dpi_cb_height/3;
    int butt_icon_wh = (int)(0.8*(double)butt_wh);
    m_ui->_TBU_closeButton->setFixedSize(butt_wh,butt_wh);
    m_ui->_TBU_closeButton->setIconSize(QSize(butt_icon_wh,butt_icon_wh));
    m_ui->_TBU_maximizeRestoreButton->setFixedSize(butt_wh,butt_wh);
    m_ui->_TBU_maximizeRestoreButton->setIconSize(QSize(butt_icon_wh,butt_icon_wh));
    m_ui->_TBU_minimizeButton->setFixedSize(butt_wh,butt_wh);
    m_ui->_TBU_minimizeButton->setIconSize(QSize(butt_icon_wh,butt_icon_wh));


}

OngoingProcessWidget::~OngoingProcessWidget()
{
    delete m_ui;
}

void OngoingProcessWidget::changeEvent(QEvent *_event)
{
    if (_event->type() == QEvent::LanguageChange)
    {
        m_ui->retranslateUi(this);
    }
}

} // namespace matisse

#include "OngoingProcessWidget.h"
#include "ui_OngoingProcessWidget.h"
#include "GraphicalCharter.h"

OngoingProcessWidget::OngoingProcessWidget(QWidget *parent) :
    QFrame(parent),
    ui(new Ui::OngoingProcessWidget)
{
    ui->setupUi(this);
}

void OngoingProcessWidget::dpiScale()
{
    MatisseCommon::GraphicalCharter &graph_charter = MatisseCommon::GraphicalCharter::instance();
    int dpi_cb_height = graph_charter.dpiScaled(CONTROLLBAR_HEIGHT);

    int dpi_stop_wh = graph_charter.dpiScaled(CB_STOP_BUTTON_ICON);
    ui->_TBU_stopButton->setFixedSize(dpi_cb_height,dpi_cb_height);
    ui->_TBU_stopButton->setIconSize(QSize(dpi_stop_wh,dpi_stop_wh));

    ui->_WID_liveProcessWheel->setFixedSize(dpi_cb_height,dpi_cb_height);

    int butt_wh = dpi_cb_height/3;
    int butt_icon_wh = (int)(0.8*(double)butt_wh);
    ui->_TBU_closeButton->setFixedSize(butt_wh,butt_wh);
    ui->_TBU_closeButton->setIconSize(QSize(butt_icon_wh,butt_icon_wh));
    ui->_TBU_maximizeRestoreButton->setFixedSize(butt_wh,butt_wh);
    ui->_TBU_maximizeRestoreButton->setIconSize(QSize(butt_icon_wh,butt_icon_wh));
    ui->_TBU_minimizeButton->setFixedSize(butt_wh,butt_wh);
    ui->_TBU_minimizeButton->setIconSize(QSize(butt_icon_wh,butt_icon_wh));


}

OngoingProcessWidget::~OngoingProcessWidget()
{
    delete ui;
}

void OngoingProcessWidget::changeEvent(QEvent *event)
{
    if (event->type() == QEvent::LanguageChange)
    {
        ui->retranslateUi(this);
    }
}

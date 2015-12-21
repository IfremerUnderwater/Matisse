#include "StatusMessageWidget.h"
#include "ui_StatusMessageWidget.h"

StatusMessageWidget::StatusMessageWidget(QWidget *parent) :
    QWidget(parent),
    _ui(new Ui::StatusMessageWidget)
{
    _ui->setupUi(this);
    connect(_ui->_PB_resetMessages, SIGNAL(clicked()), _ui->_CB_messages, SLOT(clear()));
}

StatusMessageWidget::~StatusMessageWidget()
{
    delete _ui;
}

void StatusMessageWidget::addMessage(QString message, QIcon icon)
{
    if (message.isEmpty()) {
        return;
    }
    _ui->_CB_messages->insertItem(0, icon, message);
    // on ne garde que les 20 derniers...
    if (_ui->_CB_messages->count() > 20) {
        _ui->_CB_messages->removeItem(20);
    }
    _ui->_CB_messages->setCurrentIndex(0);
}

void StatusMessageWidget::changeEvent(QEvent *event)
{
    if (event->type() == QEvent::LanguageChange)
    {
        _ui->retranslateUi(this);
    }
}

#include "status_message_widget.h"
#include "ui_status_message_widget.h"

namespace matisse {

StatusMessageWidget::StatusMessageWidget(QWidget *_parent, MatisseIconFactory *_icon_factory) :
    QWidget(_parent),
    m_ui(new Ui::StatusMessageWidget),
    m_icon_factory(_icon_factory)
{
    m_ui->setupUi(this);
    connect(m_ui->_PB_resetMessages, SIGNAL(clicked()), this, SLOT(sl_clearMessages()));
}

StatusMessageWidget::~StatusMessageWidget()
{
    delete m_ui;
}

void StatusMessageWidget::addMessage(QString _message, QString _source_icon_path, QString _color_alias)
{
    if (_message.isEmpty()) {
        qWarning() << "Trying to add empty status message";
        return;
    }

    /* increment index for all existing item wrappers */
    foreach (IconizedComboBoxItemWrapper *item_wrapper, m_item_wrappers) {
        item_wrapper->incrementItemIndex();
    }

    m_ui->_CB_messages->insertItem(0, _message);
    IconizedComboBoxItemWrapper *item_wrapper = new IconizedComboBoxItemWrapper(m_ui->_CB_messages, 0);
    if (m_icon_factory->attachIcon(item_wrapper, _source_icon_path, false, false, _color_alias)) {
        m_item_wrappers.insert(0,item_wrapper);
    }

    // we keep only last 20 messages
    if (m_ui->_CB_messages->count() > 20) {
        // remove lastItem;
        IconizedComboBoxItemWrapper *last_item = m_item_wrappers.at(20);
        m_icon_factory->detachIcon(last_item, true);
        m_ui->_CB_messages->removeItem(20);
    }
    m_ui->_CB_messages->setCurrentIndex(0);
}

void StatusMessageWidget::changeEvent(QEvent *_event)
{
    if (_event->type() == QEvent::LanguageChange)
    {
        m_ui->retranslateUi(this);
    }
}

void StatusMessageWidget::sl_clearMessages()
{
    foreach (IconizedComboBoxItemWrapper *item_wrapper, m_item_wrappers) {
        m_icon_factory->detachIcon(item_wrapper, true);
//        delete itemWrapper;
    }

    m_item_wrappers.clear();
    m_ui->_CB_messages->clear();
}

} // namespace matisse

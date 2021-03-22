#include "StatusMessageWidget.h"
#include "ui_StatusMessageWidget.h"

StatusMessageWidget::StatusMessageWidget(QWidget *parent, MatisseIconFactory *iconFactory) :
    QWidget(parent),
    _ui(new Ui::StatusMessageWidget),
    _iconFactory(iconFactory)
{
    _ui->setupUi(this);
    connect(_ui->_PB_resetMessages, SIGNAL(clicked()), this, SLOT(slot_clearMessages()));
}

StatusMessageWidget::~StatusMessageWidget()
{
    delete _ui;
}

void StatusMessageWidget::addMessage(QString message, QString sourceIconPath, QString colorAlias)
{
    if (message.isEmpty()) {
        qWarning() << "Trying to add empty status message";
        return;
    }

    /* increment index for all existing item wrappers */
    foreach (IconizedComboBoxItemWrapper *itemWrapper, _itemWrappers) {
        itemWrapper->incrementItemIndex();
    }

    _ui->_CB_messages->insertItem(0, message);
    IconizedComboBoxItemWrapper *itemWrapper = new IconizedComboBoxItemWrapper(_ui->_CB_messages, 0);
    if (_iconFactory->attachIcon(itemWrapper, sourceIconPath, false, false, colorAlias)) {
        _itemWrappers.insert(0,itemWrapper);
    }

    // we keep only last 20 messages
    if (_ui->_CB_messages->count() > 20) {
        // remove lastItem;
        IconizedComboBoxItemWrapper *lastItem = _itemWrappers.at(20);
        _iconFactory->detachIcon(lastItem, true);
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

void StatusMessageWidget::slot_clearMessages()
{
    foreach (IconizedComboBoxItemWrapper *itemWrapper, _itemWrappers) {
        _iconFactory->detachIcon(itemWrapper, true);
//        delete itemWrapper;
    }

    _itemWrappers.clear();
    _ui->_CB_messages->clear();
}

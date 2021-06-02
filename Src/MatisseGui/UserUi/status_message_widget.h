#ifndef STATUSMESSAGEWIDGET_H
#define STATUSMESSAGEWIDGET_H

#include <QWidget>
#include <QList>
#include <QtDebug>

#include "matisse_icon_factory.h"
#include "iconized_combo_box_item_wrapper.h"

using namespace matisse;

namespace Ui {
class StatusMessageWidget;
}

class StatusMessageWidget : public QWidget
{
    Q_OBJECT

public:
    explicit StatusMessageWidget(QWidget *parent, MatisseIconFactory *iconFactory);
    ~StatusMessageWidget();
    void addMessage(QString message, QString sourceIconPath, QString colorAlias);

protected:
    void changeEvent(QEvent *event); // overriding event handler for dynamic translation

protected slots:
    void slot_clearMessages();

private:
    Ui::StatusMessageWidget *_ui;
    MatisseIconFactory *_iconFactory;
    QList<IconizedComboBoxItemWrapper *> _itemWrappers;
};

#endif // STATUSMESSAGEWIDGET_H

#ifndef MATISSE_STATUS_MESSAGE_WIDGET_H_
#define MATISSE_STATUS_MESSAGE_WIDGET_H_

#include <QWidget>
#include <QList>
#include <QtDebug>

#include "matisse_icon_factory.h"
#include "iconized_combo_box_item_wrapper.h"


namespace Ui {
class StatusMessageWidget;
}

namespace matisse {

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

} // namespace matisse

#endif // MATISSE_STATUS_MESSAGE_WIDGET_H_

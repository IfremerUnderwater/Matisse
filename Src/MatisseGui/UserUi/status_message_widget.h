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
    explicit StatusMessageWidget(QWidget *_parent, MatisseIconFactory *_icon_factory);
    ~StatusMessageWidget();
    void addMessage(QString _message, QString _source_icon_path, QString _color_alias);

protected:
    void changeEvent(QEvent *_event); // overriding event handler for dynamic translation

protected slots:
    void sl_clearMessages();

private:
    Ui::StatusMessageWidget *m_ui;
    MatisseIconFactory *m_icon_factory;
    QList<IconizedComboBoxItemWrapper *> m_item_wrappers;
};

} // namespace matisse

#endif // MATISSE_STATUS_MESSAGE_WIDGET_H_

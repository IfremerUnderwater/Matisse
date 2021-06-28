#ifndef MATISSE_ICONIZED_COMBO_BOX_ITEM_WRAPPER_H_
#define MATISSE_ICONIZED_COMBO_BOX_ITEM_WRAPPER_H_

#include <QComboBox>

#include "iconized_widget_wrapper.h"

namespace matisse {

class IconizedComboBoxItemWrapper : public IconizedWidgetWrapper
{
public:
    IconizedComboBoxItemWrapper(QComboBox *_combo_box, int _initial_item_index);
    virtual void setIcon(const QIcon &_icon);
    void incrementItemIndex() { m_item_index++; }

private:
    QComboBox *m_combo_box;
    int m_item_index;
};

} // namespace matisse

#endif // MATISSE_ICONIZED_COMBO_BOX_ITEM_WRAPPER_H_

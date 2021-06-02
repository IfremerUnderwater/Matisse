#ifndef MATISSE_ICONIZED_COMBO_BOX_ITEM_WRAPPER_H_
#define MATISSE_ICONIZED_COMBO_BOX_ITEM_WRAPPER_H_

#include <QComboBox>

#include "iconized_widget_wrapper.h"

namespace matisse {

class IconizedComboBoxItemWrapper : public IconizedWidgetWrapper
{
public:
    IconizedComboBoxItemWrapper(QComboBox *comboBox, int initialItemIndex);
    virtual void setIcon(const QIcon &icon);
    void incrementItemIndex() { _itemIndex++; }

private:
    QComboBox *_comboBox;
    int _itemIndex;
};

} // namespace matisse

#endif // MATISSE_ICONIZED_COMBO_BOX_ITEM_WRAPPER_H_

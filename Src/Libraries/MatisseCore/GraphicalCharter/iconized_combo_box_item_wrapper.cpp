#include "iconized_combo_box_item_wrapper.h"

namespace matisse {

IconizedComboBoxItemWrapper::IconizedComboBoxItemWrapper(QComboBox *comboBox, int initialItemIndex) :
    IconizedWidgetWrapper(comboBox, true),
    _comboBox(comboBox),
    _itemIndex(initialItemIndex)
{

}


void IconizedComboBoxItemWrapper::setIcon(const QIcon &icon)
{
    _comboBox->setItemIcon(_itemIndex, icon);
}

} // namespace matisse

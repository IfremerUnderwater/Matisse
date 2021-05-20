#include "iconized_combo_box_item_wrapper.h"

using namespace MatisseTools;

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

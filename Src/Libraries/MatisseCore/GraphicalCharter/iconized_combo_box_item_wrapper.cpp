#include "iconized_combo_box_item_wrapper.h"

namespace matisse {

IconizedComboBoxItemWrapper::IconizedComboBoxItemWrapper(QComboBox *_combo_box, int _initial_item_index) :
    IconizedWidgetWrapper(_combo_box, true),
    m_combo_box(_combo_box),
    m_item_index(_initial_item_index)
{

}


void IconizedComboBoxItemWrapper::setIcon(const QIcon &_icon)
{
    m_combo_box->setItemIcon(m_item_index, _icon);
}

} // namespace matisse

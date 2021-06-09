#include "iconized_tree_item_wrapper.h"

namespace matisse {

IconizedTreeItemWrapper::IconizedTreeItemWrapper(MatisseTreeItem *_item, int _item_column) :
    IconizedWidgetWrapper(_item, true)
{
    m_item = _item;
    m_icon_column = _item_column;
}

void IconizedTreeItemWrapper::setIcon(const QIcon &_icon)
{
    m_item->setIcon(m_icon_column, _icon);
}

} // namespace matisse



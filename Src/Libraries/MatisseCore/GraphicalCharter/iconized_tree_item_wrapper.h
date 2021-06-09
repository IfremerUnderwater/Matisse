#ifndef MATISSE_ICONIZED_TREE_ITEM_WRAPPER_H_
#define MATISSE_ICONIZED_TREE_ITEM_WRAPPER_H_

#include "matisse_tree_item.h"
#include "iconized_widget_wrapper.h"

namespace matisse {

class IconizedTreeItemWrapper : public IconizedWidgetWrapper
{
public:
    IconizedTreeItemWrapper(MatisseTreeItem *_item, int _item_column);
    virtual void setIcon(const QIcon &_icon);

private:
    MatisseTreeItem *m_item;
    int m_icon_column;
};

} // namespace matisse

#endif // MATISSE_ICONIZED_TREE_ITEM_WRAPPER_H_

#ifndef MATISSE_ICONIZED_TREE_ITEM_WRAPPER_H_
#define MATISSE_ICONIZED_TREE_ITEM_WRAPPER_H_

#include "matisse_tree_item.h"
#include "iconized_widget_wrapper.h"

namespace matisse {

class IconizedTreeItemWrapper : public IconizedWidgetWrapper
{
public:
    IconizedTreeItemWrapper(MatisseTreeItem *item, int itemColumn);
    virtual void setIcon(const QIcon &icon);

private:
    MatisseTreeItem *_item;
    int _iconColumn;
};

} // namespace matisse

#endif // MATISSE_ICONIZED_TREE_ITEM_WRAPPER_H_

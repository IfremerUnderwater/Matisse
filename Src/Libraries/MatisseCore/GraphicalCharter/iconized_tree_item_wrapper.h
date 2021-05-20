#ifndef ICONIZEDTREEITEMWRAPPER_H
#define ICONIZEDTREEITEMWRAPPER_H

#include "matisse_tree_item.h"
#include "iconized_widget_wrapper.h"

namespace MatisseTools {
class IconizedTreeItemWrapper : public IconizedWidgetWrapper
{
public:
    IconizedTreeItemWrapper(MatisseTreeItem *item, int itemColumn);
    virtual void setIcon(const QIcon &icon);

private:
    MatisseTreeItem *_item;
    int _iconColumn;
};
}

#endif // ICONIZEDTREEITEMWRAPPER_H

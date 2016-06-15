#ifndef ICONIZEDTREEITEMWRAPPER_H
#define ICONIZEDTREEITEMWRAPPER_H

#include "MatisseTreeItem.h"
#include "IconizedWidgetWrapper.h"

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

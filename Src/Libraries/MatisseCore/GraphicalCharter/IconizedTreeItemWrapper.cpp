#include "IconizedTreeItemWrapper.h"

using namespace MatisseTools;

IconizedTreeItemWrapper::IconizedTreeItemWrapper(MatisseTreeItem *item, int iconColumn) :
    IconizedWidgetWrapper(item, true)
{
    _item = item;
    _iconColumn = iconColumn;
}

void IconizedTreeItemWrapper::setIcon(const QIcon &icon)
{
    _item->setIcon(_iconColumn, icon);
}



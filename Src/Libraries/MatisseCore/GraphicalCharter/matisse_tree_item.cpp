#include "matisse_tree_item.h"

namespace matisse {

MatisseTreeItem::MatisseTreeItem(QTreeWidgetItem *_parent, const QStringList &_strings) :
    QTreeWidgetItem(_parent, _strings, Type)
{
}

} // namespace matisse


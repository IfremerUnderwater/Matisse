#include "matisse_tree_item.h"

namespace matisse {

MatisseTreeItem::MatisseTreeItem(QTreeWidgetItem *parent, const QStringList &strings) :
    QTreeWidgetItem(parent, strings, Type)
{
}

} // namespace matisse


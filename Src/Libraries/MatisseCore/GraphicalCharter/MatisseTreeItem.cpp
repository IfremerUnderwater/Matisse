#include "MatisseTreeItem.h"

using namespace MatisseTools;

MatisseTreeItem::MatisseTreeItem(QTreeWidgetItem *parent, const QStringList &strings) :
    QTreeWidgetItem(parent, strings, Type)
{
}


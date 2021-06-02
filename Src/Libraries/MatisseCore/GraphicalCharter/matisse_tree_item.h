#ifndef MATISSE_MATISSE_TREE_ITEM_H_
#define MATISSE_MATISSE_TREE_ITEM_H_

#include <QObject>
#include <QTreeWidgetItem>

namespace matisse {

/* Subclassing QTreeWidgetItem to add QObject inheritance */
class MatisseTreeItem : public QTreeWidgetItem, public QObject
{
public:
    enum { Type = QTreeWidgetItem::UserType + 1 };

    MatisseTreeItem(QTreeWidgetItem *parent, const QStringList & strings);
};

} // namespace matisse

#endif // MATISSE_MATISSE_TREE_ITEM_H_

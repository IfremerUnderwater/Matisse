#ifndef MATISSETREEITEM_H
#define MATISSETREEITEM_H

#include <QObject>
#include <QTreeWidgetItem>

namespace MatisseTools {

/* Subclassing QTreeWidgetItem to add QObject inheritance */
class MatisseTreeItem : public QTreeWidgetItem, public QObject
{
public:
    enum { Type = QTreeWidgetItem::UserType + 1 };

    MatisseTreeItem(QTreeWidgetItem *parent, const QStringList & strings);
};
}

#endif // MATISSETREEITEM_H

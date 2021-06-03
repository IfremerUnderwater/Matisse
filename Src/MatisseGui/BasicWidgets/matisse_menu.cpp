#include "matisse_menu.h"

namespace matisse {

MatisseMenu::MatisseMenu(QWidget* parent) : QMenu(parent)
{
}

MatisseMenu::MatisseMenu(const QString &title, QWidget *parent) : QMenu(title, parent)
{
}

void MatisseMenu::showEvent(QShowEvent *event)
{
    Q_UNUSED(event)

    QWidget* parent = parentWidget();
    QPoint p = this->pos();
    this->move(p.x(), parent->mapToGlobal(parent->pos()).y() + parent->height());
}

} // namespace matisse

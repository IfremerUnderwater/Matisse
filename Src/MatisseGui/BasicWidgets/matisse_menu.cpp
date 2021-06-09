#include "matisse_menu.h"

namespace matisse {

MatisseMenu::MatisseMenu(QWidget* _parent) : QMenu(_parent)
{
}

MatisseMenu::MatisseMenu(const QString &_title, QWidget *_parent) : QMenu(_title, _parent)
{
}

void MatisseMenu::showEvent(QShowEvent *_event)
{
    Q_UNUSED(_event)

    QWidget* parent = parentWidget();
    QPoint p = this->pos();
    this->move(p.x(), parent->mapToGlobal(parent->pos()).y() + parent->height());
}

} // namespace matisse

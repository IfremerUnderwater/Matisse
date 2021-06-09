#ifndef MATISSE_MATISSE_MENU_H_
#define MATISSE_MATISSE_MENU_H_

#include <QWidget>
#include <QMenu>
#include <QShowEvent>
#include <QString>

namespace matisse {

class MatisseMenu : public QMenu
{
public:
    MatisseMenu(QWidget* _parent = 0);
    MatisseMenu(const QString & _title, QWidget * _parent = 0);
    void showEvent(QShowEvent * _event);
};

} // namespace matisse

#endif // MATISSE_MATISSE_MENU_H_

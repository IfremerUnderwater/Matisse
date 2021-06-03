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
    MatisseMenu(QWidget* parent = 0);
    MatisseMenu(const QString & title, QWidget * parent = 0);
    void showEvent(QShowEvent * event);
};

} // namespace matisse

#endif // MATISSE_MATISSE_MENU_H_

#ifndef MATISSEMENU_H
#define MATISSEMENU_H

#include <QWidget>
#include <QMenu>
#include <QShowEvent>
#include <QString>

class MatisseMenu : public QMenu
{
public:
    MatisseMenu(QWidget* parent = 0);
    MatisseMenu(const QString & title, QWidget * parent = 0);
    void showEvent(QShowEvent * event);
};


#endif // MATISSEMENU_H

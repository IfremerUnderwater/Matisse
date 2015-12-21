#ifndef MATISSETABWIDGET_H
#define MATISSETABWIDGET_H

#include <QTabWidget>
#include <QStackedLayout>

class MatisseTabWidget : public QTabWidget
{
public:
    MatisseTabWidget(QWidget* parent);
    ~MatisseTabWidget();

private:
//    QStackedLayout* _stackedLayout;
//    QTabBar* _tabBar;
};

#endif // MATISSETABWIDGET_H

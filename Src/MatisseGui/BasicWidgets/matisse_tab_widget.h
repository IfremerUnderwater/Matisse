#ifndef MATISSE_MATISSE_TAB_WIDGET_H_
#define MATISSE_MATISSE_TAB_WIDGET_H_

#include <QTabWidget>
#include <QStackedLayout>

namespace matisse {

class MatisseTabWidget : public QTabWidget
{
public:
    MatisseTabWidget(QWidget* parent);
    ~MatisseTabWidget();

    void setObjectName(const QString &name);
private:
//    QStackedLayout* _stackedLayout;
//    QTabBar* _tabBar;
};

} // namespace matisse

#endif // MATISSE_MATISSE_TAB_WIDGET_H_

#include "matisse_tab_widget.h"

#include <QTabBar>
#include <QtDebug>

namespace matisse {

MatisseTabWidget::MatisseTabWidget(QWidget* _parent) :
    QTabWidget(_parent)
{

}

MatisseTabWidget::~MatisseTabWidget()
{
}

void MatisseTabWidget::setObjectName(const QString &_name)
{
    QObject::setObjectName(_name);
    tabBar()->setObjectName(_name + "-tabbar");
}

} // namespace matisse



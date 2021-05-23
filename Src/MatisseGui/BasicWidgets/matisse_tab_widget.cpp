#include "matisse_tab_widget.h"

#include <QTabBar>
#include <QtDebug>

MatisseTabWidget::MatisseTabWidget(QWidget* parent) :
    QTabWidget(parent)
{
//    QTabBar* _tabBar = new QTabBar(this);
//    _tabBar->setExpanding(true);
//    setTabBar(_tabBar);
//    QVBoxLayout* layout = new QVBoxLayout(this);
//    setLayout(layout);
//    _stackedLayout = new QStackedLayout();
//    layout->addWidget(_tabBar);
//    layout->addLayout(_stackedLayout);
//    connect(_tabBar, SIGNAL(currentChanged(int)), _stackedLayout, SLOT(setCurrentIndex(int)));

//    qDebug() << "Expanding";
}

MatisseTabWidget::~MatisseTabWidget()
{
}

void MatisseTabWidget::setObjectName(const QString &name)
{
    QObject::setObjectName(name);
    tabBar()->setObjectName(name + "-tabbar");
}



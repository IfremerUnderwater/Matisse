#include "home_widget.h"
#include "ui_home_widget.h"

#include <QtDebug>

namespace matisse {

HomeWidget::HomeWidget(QWidget *_parent) :
    QFrame(_parent),
    m_ui(new Ui::HomeWidget)
{
    m_ui->setupUi(this);
    connect(m_ui->_TB_homeButton, SIGNAL(clicked()), this, SLOT(sl_goHome()));
}

HomeWidget::~HomeWidget()
{
    delete m_ui;
}

void HomeWidget::sl_goHome()
{
    emit si_goHome();
}

} // namespace matisse

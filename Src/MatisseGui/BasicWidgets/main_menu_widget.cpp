#include "main_menu_widget.h"
#include "ui_main_menu_widget.h"
#include "graphical_charter.h"

#include <QVBoxLayout>
#include <QMenuBar>
#include <QMenu>
#include <QVBoxLayout>

namespace matisse {

MainMenuWidget::MainMenuWidget(QWidget *_parent) :
    QFrame(_parent),
    m_ui(new Ui::MainMenuWidget)
{
    m_ui->setupUi(this);

    QVBoxLayout *box_layout = new QVBoxLayout(this); // Main layout of widget
    box_layout->setAlignment(Qt::AlignVCenter);
    box_layout->setContentsMargins(0,0,0,0);
    setLayout(box_layout);

    QMenuBar* menu_bar = new QMenuBar(this);
    menu_bar->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Fixed);
    menu_bar->setObjectName("_MBA_mainMenuBar");
    this->layout()->addWidget(menu_bar);

}

MainMenuWidget::~MainMenuWidget()
{
    delete m_ui;
}

void MainMenuWidget::changeEvent(QEvent *_event)
{
    if (_event->type() == QEvent::LanguageChange)
    {
        m_ui->retranslateUi(this);
    }
}

} // namespace matisse

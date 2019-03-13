#include "AboutWebView.h"

AboutWebView::AboutWebView(QWidget* parent) : QDialog(parent)
{
    QLayout *layout = new QVBoxLayout(this);
    QWebView *view = new QWebView(this);
    setLayout(layout);
    layout->addWidget(view);
    view->load(QUrl("help/matisse_about_FR.html"));
    view->show();
}

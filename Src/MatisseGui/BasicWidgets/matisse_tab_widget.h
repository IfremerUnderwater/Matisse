#ifndef MATISSE_MATISSE_TAB_WIDGET_H_
#define MATISSE_MATISSE_TAB_WIDGET_H_

#include <QTabWidget>
#include <QStackedLayout>

namespace matisse {

class MatisseTabWidget : public QTabWidget
{
public:
    MatisseTabWidget(QWidget* _parent);
    ~MatisseTabWidget();

    void setObjectName(const QString &_name);
private:

};

} // namespace matisse

#endif // MATISSE_MATISSE_TAB_WIDGET_H_

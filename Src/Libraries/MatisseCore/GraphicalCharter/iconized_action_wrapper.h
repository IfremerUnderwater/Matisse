#ifndef MATISSE_ICONIZED_ACTION_WRAPPER_H_
#define MATISSE_ICONIZED_ACTION_WRAPPER_H_

#include <QAction>

#include "iconized_widget_wrapper.h"

namespace matisse {

class IconizedActionWrapper : public IconizedWidgetWrapper
{
public:
    IconizedActionWrapper(QAction *action);
    virtual void setIcon(const QIcon &icon);

private:
    QAction *_action;
};

} // namespace matisse

#endif // MATISSE_ICONIZED_ACTION_WRAPPER_H_

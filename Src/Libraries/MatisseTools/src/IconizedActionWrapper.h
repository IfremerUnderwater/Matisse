#ifndef ICONIZEDACTIONWRAPPER_H
#define ICONIZEDACTIONWRAPPER_H

#include <QAction>

#include "IconizedWidgetWrapper.h"

namespace MatisseTools {
class IconizedActionWrapper : public IconizedWidgetWrapper
{
public:
    IconizedActionWrapper(QAction *action);
    virtual void setIcon(const QIcon &icon);

private:
    QAction *_action;
};
}

#endif // ICONIZEDACTIONWRAPPER_H

#include "iconized_action_wrapper.h"

using namespace MatisseTools;

IconizedActionWrapper::IconizedActionWrapper(QAction *action) :
    IconizedWidgetWrapper(action, true),
    _action(action)
{

}

void IconizedActionWrapper::setIcon(const QIcon &icon)
{
    _action->setIcon(icon);
}


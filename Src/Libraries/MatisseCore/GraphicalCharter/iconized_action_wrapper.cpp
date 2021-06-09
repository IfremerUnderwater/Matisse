#include "iconized_action_wrapper.h"

namespace matisse {

IconizedActionWrapper::IconizedActionWrapper(QAction *_action) :
    IconizedWidgetWrapper(_action, true),
    m_action(_action)
{

}

void IconizedActionWrapper::setIcon(const QIcon &_icon)
{
    m_action->setIcon(_icon);
}

} // namespace matisse


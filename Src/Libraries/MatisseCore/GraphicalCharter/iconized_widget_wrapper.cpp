#include "iconized_widget_wrapper.h"

namespace matisse {

IconizedWidgetWrapper::IconizedWidgetWrapper(QObject *_widget, bool _has_icon) :
    m_has_icon(_has_icon),
    m_is_valid(true),
    m_widget(_widget)
{
    if (m_widget) {
        connect(m_widget, SIGNAL(destroyed(QObject*)), this, SLOT(sl_invalidate(QObject*)));
    }
}


bool IconizedWidgetWrapper::hasIcon() const
{
    return m_has_icon;
}

void IconizedWidgetWrapper::sl_invalidate(QObject *_widget)
{
    Q_UNUSED(_widget)
    m_is_valid = false;
}

bool IconizedWidgetWrapper::isValid() const
{
    return m_is_valid;
}

} // namespace matisse


#include "IconizedWidgetWrapper.h"

using namespace MatisseTools;

IconizedWidgetWrapper::IconizedWidgetWrapper(QObject *widget, bool hasIcon) :
    _hasIcon(hasIcon),
    _isValid(true),
    _widget(widget)
{
    if (_widget) {
        connect(_widget, SIGNAL(destroyed(QObject*)), this, SLOT(slot_invalidate(QObject*)));
    }
}


bool IconizedWidgetWrapper::hasIcon() const
{
    return _hasIcon;
}

void IconizedWidgetWrapper::slot_invalidate(QObject *widget)
{
    Q_UNUSED(widget)
    _isValid = false;
}

bool IconizedWidgetWrapper::isValid() const
{
    return _isValid;
}


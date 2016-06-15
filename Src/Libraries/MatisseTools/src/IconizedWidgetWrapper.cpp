#include "IconizedWidgetWrapper.h"

using namespace MatisseTools;

IconizedWidgetWrapper::IconizedWidgetWrapper(QObject *widget, bool hasIcon) :
    _widget(widget),
    _hasIcon(hasIcon),
    _isValid(true)
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
    _isValid = false;
}

bool IconizedWidgetWrapper::isValid() const
{
    return _isValid;
}


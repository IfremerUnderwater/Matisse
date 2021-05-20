#include "iconized_button_wrapper.h"

using namespace MatisseTools;

IconizedButtonWrapper::IconizedButtonWrapper(QAbstractButton *button) :
    IconizedWidgetWrapper(button, true),
    _button(button)
{

}

void IconizedButtonWrapper::setIcon(const QIcon &icon)
{
    _button->setIcon(icon);
}


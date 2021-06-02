#include "iconized_button_wrapper.h"

namespace matisse {

IconizedButtonWrapper::IconizedButtonWrapper(QAbstractButton *button) :
    IconizedWidgetWrapper(button, true),
    _button(button)
{

}

void IconizedButtonWrapper::setIcon(const QIcon &icon)
{
    _button->setIcon(icon);
}

} // namespace matisse

#include "iconized_button_wrapper.h"

namespace matisse {

IconizedButtonWrapper::IconizedButtonWrapper(QAbstractButton *_button) :
    IconizedWidgetWrapper(_button, true),
    m_button(_button)
{

}

void IconizedButtonWrapper::setIcon(const QIcon &_icon)
{
    m_button->setIcon(_icon);
}

} // namespace matisse

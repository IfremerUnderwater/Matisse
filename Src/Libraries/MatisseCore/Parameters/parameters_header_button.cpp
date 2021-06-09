#include "parameters_header_button.h"

namespace matisse {

ParametersHeaderButton::ParametersHeaderButton(QWidget *_parent, eParameterLevel _level) :
    FoldUnfoldButton(_parent),
    m_level(_level)
{
    setIcon(QIcon(":/qss_icons/rc/branch_open-on.png"));
}

void ParametersHeaderButton::showNewState()
{
    if (getIsUnfolded()) {
        setIcon(QIcon(":/qss_icons/rc/branch_open-on.png"));
    } else {
        setIcon(QIcon(":/qss_icons/rc/branch_closed-on.png"));
    }
}

eParameterLevel ParametersHeaderButton::getLevel() const
{
    return m_level;
}

} // namespace matisse




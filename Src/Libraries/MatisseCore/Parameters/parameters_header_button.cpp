#include "parameters_header_button.h"

using namespace MatisseTools;

ParametersHeaderButton::ParametersHeaderButton(QWidget *parent, ParameterLevel level) :
    FoldUnfoldButton(parent),
    _level(level)
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

ParameterLevel ParametersHeaderButton::getLevel() const
{
    return _level;
}




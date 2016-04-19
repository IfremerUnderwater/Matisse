#ifndef PARAMETERSHEADERBUTTON_H
#define PARAMETERSHEADERBUTTON_H

#include <QPushButton>
#include "FoldUnfoldButton.h"
#include "ToolsCommon.h"


namespace MatisseTools {

class ParametersHeaderButton : public FoldUnfoldButton
{
    Q_OBJECT

public:
    ParametersHeaderButton(QWidget* parent, ParameterLevel level);

    ParameterLevel getLevel() const;

protected:
    virtual void showNewState();

private:
    ParameterLevel _level;
};

}

#endif // PARAMETERSHEADERBUTTON_H

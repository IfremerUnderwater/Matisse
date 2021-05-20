#ifndef PARAMETERSHEADERBUTTON_H
#define PARAMETERSHEADERBUTTON_H

#include <QPushButton>
#include "fold_unfold_button.h"
#include "parameters_common.h"


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

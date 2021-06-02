#ifndef MATISSE_PARAMETERS_HEADER_BUTTON_H_
#define MATISSE_PARAMETERS_HEADER_BUTTON_H_

#include <QPushButton>
#include "fold_unfold_button.h"
#include "parameters_common.h"


namespace matisse {

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

} // namespace matisse

#endif // MATISSE_PARAMETERS_HEADER_BUTTON_H_

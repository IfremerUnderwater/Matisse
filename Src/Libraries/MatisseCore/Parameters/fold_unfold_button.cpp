#include "fold_unfold_button.h"

namespace matisse {

FoldUnfoldButton::FoldUnfoldButton(QWidget *parent) : QPushButton(parent),
    _isUnfolded(false)
{
    connect(this, SIGNAL(clicked(bool)), this, SLOT(flip()));
}

void FoldUnfoldButton::setIsUnfolded(bool isUnfolded)
{
    _isUnfolded = isUnfolded;
    showNewState();
}

bool FoldUnfoldButton::getIsUnfolded() const
{
    return _isUnfolded;
}

void FoldUnfoldButton::flip()
{
    setIsUnfolded(!_isUnfolded);
}

} // namespace matisse



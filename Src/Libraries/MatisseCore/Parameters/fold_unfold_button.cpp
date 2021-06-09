#include "fold_unfold_button.h"

namespace matisse {

FoldUnfoldButton::FoldUnfoldButton(QWidget *_parent) : QPushButton(_parent),
    m_is_unfolded(false)
{
    connect(this, SIGNAL(clicked(bool)), this, SLOT(sl_flip()));
}

void FoldUnfoldButton::setIsUnfolded(bool _is_unfolded)
{
    m_is_unfolded = _is_unfolded;
    showNewState();
}

bool FoldUnfoldButton::getIsUnfolded() const
{
    return m_is_unfolded;
}

void FoldUnfoldButton::sl_flip()
{
    setIsUnfolded(!m_is_unfolded);
}

} // namespace matisse



#ifndef MATISSE_FOLD_UNFOLD_BUTTON_H_
#define MATISSE_FOLD_UNFOLD_BUTTON_H_

#include <QPushButton>

namespace matisse {

class FoldUnfoldButton : public QPushButton
{
    Q_OBJECT

public:
    FoldUnfoldButton(QWidget *_parent=0);

    bool getIsUnfolded() const;
    void setIsUnfolded(bool _is_unfolded);

protected:
    virtual void showNewState() = 0;

public slots:
    void sl_flip();

private:
    bool m_is_unfolded;
};

}

#endif // MATISSE_FOLD_UNFOLD_BUTTON_H_

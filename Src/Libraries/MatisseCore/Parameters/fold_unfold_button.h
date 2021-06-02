#ifndef MATISSE_FOLD_UNFOLD_BUTTON_H_
#define MATISSE_FOLD_UNFOLD_BUTTON_H_

#include <QPushButton>

namespace matisse {

class FoldUnfoldButton : public QPushButton
{
    Q_OBJECT

public:
    FoldUnfoldButton(QWidget *parent=0);

    bool getIsUnfolded() const;
    void setIsUnfolded(bool isUnfolded);

protected:
    virtual void showNewState() = 0;

public slots:
    void flip();

private:
    bool _isUnfolded;
};

}

#endif // MATISSE_FOLD_UNFOLD_BUTTON_H_

#ifndef FOLDUNFOLDBUTTON_H
#define FOLDUNFOLDBUTTON_H

#include <QPushButton>

namespace MatisseTools {

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

#endif // FOLDUNFOLDBUTTON_H

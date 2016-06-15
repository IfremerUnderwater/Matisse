#ifndef ICONIZEDBUTTONWRAPPER_H
#define ICONIZEDBUTTONWRAPPER_H

#include <QAbstractButton>

#include "IconizedWidgetWrapper.h"

namespace MatisseTools {
class IconizedButtonWrapper : public IconizedWidgetWrapper
{
public:
    IconizedButtonWrapper(QAbstractButton *button);

    virtual void setIcon(const QIcon &icon);

private:
    QAbstractButton *_button;
};
}

#endif // ICONIZEDBUTTONWRAPPER_H

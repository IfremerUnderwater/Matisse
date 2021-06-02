#ifndef MATISSE_ICONIZED_BUTTON_WRAPPER_H_
#define MATISSE_ICONIZED_BUTTON_WRAPPER_H_

#include <QAbstractButton>

#include "iconized_widget_wrapper.h"

namespace matisse {

class IconizedButtonWrapper : public IconizedWidgetWrapper
{
public:
    IconizedButtonWrapper(QAbstractButton *button);

    virtual void setIcon(const QIcon &icon);

private:
    QAbstractButton *_button;
};

} // namespace matisse

#endif // MATISSE_ICONIZED_BUTTON_WRAPPER_H_

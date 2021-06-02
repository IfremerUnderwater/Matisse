#ifndef MATISSE_ICONIZED_LABEL_WRAPPER_H_
#define MATISSE_ICONIZED_LABEL_WRAPPER_H_

#include <QLabel>

#include "iconized_widget_wrapper.h"

namespace matisse {

class IconizedLabelWrapper : public IconizedWidgetWrapper
{
public:
    IconizedLabelWrapper(QLabel *label);

    virtual void setPixmap(const QPixmap &pixmap);

private:
    QLabel *_label;
};

} // namespace matisse

#endif // MATISSE_ICONIZED_LABEL_WRAPPER_H_

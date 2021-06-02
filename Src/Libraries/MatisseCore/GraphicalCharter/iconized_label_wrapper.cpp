#include "iconized_label_wrapper.h"

namespace matisse {

IconizedLabelWrapper::IconizedLabelWrapper(QLabel *label) :
    IconizedWidgetWrapper(label, false),
    _label(label)
{
}

void IconizedLabelWrapper::setPixmap(const QPixmap &pixmap)
{
    _label->setPixmap(pixmap);
}

} // namespace matisse


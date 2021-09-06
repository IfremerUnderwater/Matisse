#include "iconized_label_wrapper.h"

namespace matisse {

IconizedLabelWrapper::IconizedLabelWrapper(QLabel *_label) :
    IconizedWidgetWrapper(_label, false),
    m_label(_label)
{
}

void IconizedLabelWrapper::setPixmap(const QPixmap &_pixmap)
{
    m_label->setPixmap(_pixmap);
}

} // namespace matisse


#ifndef MATISSE_ICONIZED_LABEL_WRAPPER_H_
#define MATISSE_ICONIZED_LABEL_WRAPPER_H_

#include <QLabel>

#include "iconized_widget_wrapper.h"

namespace matisse {

class IconizedLabelWrapper : public IconizedWidgetWrapper
{
public:
    IconizedLabelWrapper(QLabel *_label);

    virtual void setPixmap(const QPixmap &_pixmap);

private:
    QLabel *m_label;
};

} // namespace matisse

#endif // MATISSE_ICONIZED_LABEL_WRAPPER_H_

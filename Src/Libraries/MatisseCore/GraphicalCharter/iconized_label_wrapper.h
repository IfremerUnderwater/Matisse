#ifndef ICONIZEDLABELWRAPPER_H
#define ICONIZEDLABELWRAPPER_H

#include <QLabel>

#include "iconized_widget_wrapper.h"

namespace MatisseTools {

class IconizedLabelWrapper : public IconizedWidgetWrapper
{
public:
    IconizedLabelWrapper(QLabel *label);

    virtual void setPixmap(const QPixmap &pixmap);

private:
    QLabel *_label;
};

}

#endif // ICONIZEDLABELWRAPPER_H

#include "IconizedLabelWrapper.h"

using namespace MatisseTools;

IconizedLabelWrapper::IconizedLabelWrapper(QLabel *label) :
    IconizedWidgetWrapper(label, false),
    _label(label)
{
}

void IconizedLabelWrapper::setPixmap(const QPixmap &pixmap)
{
    _label->setPixmap(pixmap);
}




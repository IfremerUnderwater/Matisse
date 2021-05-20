#ifndef ICONIZEDCOMBOBOXITEMWRAPPER_H
#define ICONIZEDCOMBOBOXITEMWRAPPER_H

#include <QComboBox>

#include "iconized_widget_wrapper.h"

namespace MatisseTools {
class IconizedComboBoxItemWrapper : public IconizedWidgetWrapper
{
public:
    IconizedComboBoxItemWrapper(QComboBox *comboBox, int initialItemIndex);
    virtual void setIcon(const QIcon &icon);
    void incrementItemIndex() { _itemIndex++; }

private:
    QComboBox *_comboBox;
    int _itemIndex;
};
}

#endif // ICONIZEDCOMBOBOXITEMWRAPPER_H

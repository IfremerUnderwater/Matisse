#ifndef ENRICHEDLISTBOX_H
#define ENRICHEDLISTBOX_H

#include <QListWidget>
#include <QStringList>
#include <QtDebug>

#include "EnrichedFormWidget.h"

namespace MatisseTools {

class EnrichedListBox : public EnrichedFormWidget
{
    Q_OBJECT
public:
    explicit EnrichedListBox(QWidget *parent, QString label, QStringList values, QString defaultValue);
    bool currentValueChanged();
    virtual QString currentValue();
    virtual void restoreDefaultValue();
    virtual void setValue(QString newValue);

signals:

public slots:

private:
    QListWidget * _list;
    int _defaultValueIndex;
};
}
#endif // ENRICHEDLISTBOX_H

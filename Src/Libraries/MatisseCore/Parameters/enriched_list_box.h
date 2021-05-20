#ifndef ENRICHEDLISTBOX_H
#define ENRICHEDLISTBOX_H

#include <QListWidget>
#include <QStringList>
#include <QtDebug>

#include "enriched_form_widget.h"

namespace MatisseTools {

class EnrichedListBox : public EnrichedFormWidget
{
    Q_OBJECT
public:
    explicit EnrichedListBox(QWidget *parent, QString label, QStringList values, QString defaultValue);
    //bool currentValueChanged();
    virtual QString currentValue();
    virtual void restoreDefaultValue();

protected:
    virtual void applyValue(QString newValue);

signals:

public slots:

private:
    QListWidget * _list;
    int _defaultValueIndex;
};
}
#endif // ENRICHEDLISTBOX_H

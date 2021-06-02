#ifndef MATISSE_ENRICHED_LIST_BOX_H_
#define MATISSE_ENRICHED_LIST_BOX_H_

#include <QListWidget>
#include <QStringList>
#include <QtDebug>

#include "enriched_form_widget.h"

namespace matisse {

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
#endif // MATISSE_ENRICHED_LIST_BOX_H_

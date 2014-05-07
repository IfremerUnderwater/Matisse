#ifndef ENRICHEDLISTBOX_H
#define ENRICHEDLISTBOX_H

#include <QListWidget>
#include <QStringList>

#include "EnrichedFormWidget.h"

namespace MatisseTools {

class EnrichedListBox : public EnrichedFormWidget
{
    Q_OBJECT
public:
    explicit EnrichedListBox(QWidget *parent, QString label, QStringList values, QString defaultValue);
    bool currentValueChanged();
    virtual QString currentValue();

signals:

public slots:

private:
    QListWidget * _list;
};
}
#endif // ENRICHEDLISTBOX_H

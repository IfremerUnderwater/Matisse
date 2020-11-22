#ifndef ENRICHEDCAMCOMBOBOX_H
#define ENRICHEDCAMCOMBOBOX_H

#include <QComboBox>
#include <QStringList>
#include <QtDebug>

#include "EnrichedFormWidget.h"

namespace MatisseTools {

class EnrichedCamComboBox : public EnrichedFormWidget
{
    Q_OBJECT
public:
    explicit EnrichedCamComboBox(QWidget *parent = 0, QString label = "", QString defaultValue = "");

    bool currentValueChanged();
    virtual QString currentValue();
    qint32 currentIndex();
    virtual void restoreDefaultValue();

protected:
    virtual void applyValue(QString newValue);

signals:

public slots:

private:
        QComboBox *_combo;
        qint32 _defaultIndex;
        qint32 _initialIndex;
};
}
#endif // ENRICHEDCAMCOMBOBOX_H

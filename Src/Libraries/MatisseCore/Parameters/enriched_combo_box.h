#ifndef MATISSE_ENRICHED_COMBO_BOX_H_
#define MATISSE_ENRICHED_COMBO_BOX_H_

#include <QComboBox>
#include <QStringList>
#include <QtDebug>

#include "enriched_form_widget.h"

namespace matisse {

class EnrichedComboBox : public EnrichedFormWidget
{
    Q_OBJECT
public:
    explicit EnrichedComboBox(QWidget *parent = 0, QString label = "", QStringList values = QStringList(), QString defaultValue = "");

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

} // namespace matisse

#endif // MATISSE_ENRICHED_COMBO_BOX_H_

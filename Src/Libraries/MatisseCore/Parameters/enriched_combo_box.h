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
    explicit EnrichedComboBox(QWidget *_parent = 0, QString m_label = "", QStringList _values = QStringList(), QString _default_value = "");

    bool currentValueChanged();
    virtual QString currentValue();
    qint32 currentIndex();
    virtual void restoreDefaultValue();

protected:
    virtual void applyValue(QString _new_value);

signals:

public slots:

private:
        QComboBox *m_combo;
        qint32 m_default_index;
        qint32 m_initial_index;
};

} // namespace matisse

#endif // MATISSE_ENRICHED_COMBO_BOX_H_

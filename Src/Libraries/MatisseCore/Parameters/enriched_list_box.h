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
    explicit EnrichedListBox(QWidget *_parent, QString _label, QStringList _values, QString _default_value);
    virtual QString currentValue();
    virtual void restoreDefaultValue();

protected:
    virtual void applyValue(QString _new_value);

signals:

public slots:

private:
    QListWidget * m_list;
    int m_default_value_index;
};
}
#endif // MATISSE_ENRICHED_LIST_BOX_H_

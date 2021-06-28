#ifndef MATISSE_ENRICHED_LINE_EDIT_H_
#define MATISSE_ENRICHED_LINE_EDIT_H_

#include <QLineEdit>
#include <QString>

#include "enriched_form_widget.h"

namespace matisse {

class EnrichedLineEdit : public EnrichedFormWidget
{
    Q_OBJECT
public:
    explicit EnrichedLineEdit(QWidget *_parent = 0, QString _label = "", QString _default_value = "");
    virtual QString currentValue();
    virtual void restoreDefaultValue();

protected:
    virtual void applyValue(QString _new_value);

signals:

public slots:

private:
    QLineEdit * m_line_edit;

};

} // namespace matisse

#endif // MATISSE_ENRICHED_LINE_EDIT_H_

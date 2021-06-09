#include "enriched_line_edit.h"

namespace matisse {

EnrichedLineEdit::EnrichedLineEdit(QWidget *_parent, QString _label, QString _default_value):
    EnrichedFormWidget(_parent)
{
    m_line_edit = new QLineEdit(_default_value, this);
    m_default_value = _default_value;
    setWidget(_label, m_line_edit);
    connect(m_line_edit, SIGNAL(textEdited(QString)), this, SLOT(sl_valueChanged()));
}


QString EnrichedLineEdit::currentValue()
{
    return m_line_edit->text().trimmed();
}

void EnrichedLineEdit::restoreDefaultValue()
{
    disconnect(m_line_edit, SIGNAL(textEdited(QString)), this, SLOT(sl_valueChanged()));
    m_line_edit->setText(m_default_value);
    connect(m_line_edit, SIGNAL(textEdited(QString)), this, SLOT(sl_valueChanged()));
}

void EnrichedLineEdit::applyValue(QString _new_value)
{
    disconnect(m_line_edit, SIGNAL(textEdited(QString)), this, SLOT(sl_valueChanged()));
    m_line_edit->setText(_new_value);
    connect(m_line_edit, SIGNAL(textEdited(QString)), this, SLOT(sl_valueChanged()));
}


} // namespace matisse

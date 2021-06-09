#ifndef MATISSE_ENRICHED_FILE_CHOOSER_H_
#define MATISSE_ENRICHED_FILE_CHOOSER_H_

#include <QWidget>
#include <QHBoxLayout>
#include <QPushButton>
#include <QLineEdit>
#include <QFileDialog>

#include "parameters_common.h"
#include "enriched_form_widget.h"
#include "matisse_icon_factory.h"
#include "iconized_button_wrapper.h"

namespace matisse {

//    enum FileChooserType{
//        DIR,
//        FILE
//    };

class EnrichedFileChooser : public EnrichedFormWidget
{
    Q_OBJECT
public:
    explicit EnrichedFileChooser(QWidget *_parent, MatisseIconFactory *_icon_factory, QString m_label, eParameterShow _type, QString _default_value);
    virtual QString currentValue();
    virtual void restoreDefaultValue();

protected:
    virtual void applyValue(QString _new_value);

private:
    QLineEdit * m_line_edit;
    eParameterShow m_type;

signals:

public slots:
    void sl_clicked();

};

} // namespace matisse

#endif // MATISSE_ENRICHED_FILE_CHOOSER_H_

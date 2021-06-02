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
    explicit EnrichedFileChooser(QWidget *parent, MatisseIconFactory *iconFactory, QString label, ParameterShow type, QString defaultValue);
    //bool currentValueChanged();
    virtual QString currentValue();
    virtual void restoreDefaultValue();

protected:
    virtual void applyValue(QString newValue);

private:
    QLineEdit * _lineEdit;
    ParameterShow _type;

signals:

public slots:
    void slot_clicked();

};

} // namespace matisse

#endif // MATISSE_ENRICHED_FILE_CHOOSER_H_

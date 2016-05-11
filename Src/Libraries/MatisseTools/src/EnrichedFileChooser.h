#ifndef ENRICHEDFILECHOOSER_H
#define ENRICHEDFILECHOOSER_H

#include <QWidget>
#include <QHBoxLayout>
#include <QPushButton>
#include <QLineEdit>
#include <QFileDialog>

#include "ToolsCommon.h"
#include "EnrichedFormWidget.h"

namespace MatisseTools {

//    enum FileChooserType{
//        DIR,
//        FILE
//    };

class EnrichedFileChooser : public EnrichedFormWidget
{
    Q_OBJECT
public:
    explicit EnrichedFileChooser(QWidget *parent, QString label, ParameterShow type, QString defaultValue);
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
}
#endif // ENRICHEDFILECHOOSER_H

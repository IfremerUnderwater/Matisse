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
    explicit EnrichedFileChooser(QWidget *parent, QString label, QString buttonLabel, ParameterShow type, QString defaultValue);
    bool currentValueChanged();
    virtual QString currentValue();

private:
    QLineEdit * _lineEdit;
    ParameterShow _type;

signals:

public slots:
    void slot_clicked();

};
}
#endif // ENRICHEDFILECHOOSER_H

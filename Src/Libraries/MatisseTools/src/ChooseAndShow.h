#ifndef CHOOSEANDSHOW_H
#define CHOOSEANDSHOW_H

#include <QWidget>
#include <QString>
#include <QFileDialog>

#include "EnrichedFormWidget.h"

namespace Ui {
class ChooseAndShow;
}

namespace MatisseTools {
class ChooseAndShow : public EnrichedFormWidget
{
    Q_OBJECT
    
public:
    explicit ChooseAndShow(QWidget *parent, QString label, QString buttonText, QString defaultValue);
    ~ChooseAndShow();
    bool currentValueChanged();
    virtual QString currentValue();
    QString buttonText();

protected slots:
    void slot_clicked();

signals:
    void signal_clicked();

private:
    Ui::ChooseAndShow *_ui;
};
}

#endif // CHOOSEANDSHOW_H

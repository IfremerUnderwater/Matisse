#ifndef MATISSESPINBOX_H
#define MATISSESPINBOX_H

#include <QAbstractSpinBox>

namespace MatisseServer {
class MatisseSpinBox : public QAbstractSpinBox
{
    Q_OBJECT
public:
    explicit MatisseSpinBox(QWidget *parent = 0);
    QString getValue();
    void setMaxRangeIsInf();



signals:

public slots:

};
}

#endif // MATISSESPINBOX_H

#ifndef CHOOSEANDSHOW_H
#define CHOOSEANDSHOW_H

#include <QWidget>
#include <QString>

namespace Ui {
class ChooseAndShow;
}

class ChooseAndShow : public QWidget
{
    Q_OBJECT
    
public:
    explicit ChooseAndShow(QString buttonText, QWidget *parent = 0);
    ~ChooseAndShow();
    void setText(QString text);
    QString text();
    QString buttonText();

protected slots:
    void slot_clicked();

signals:
    void signal_clicked();

private:
    Ui::ChooseAndShow *ui;
};

#endif // CHOOSEANDSHOW_H

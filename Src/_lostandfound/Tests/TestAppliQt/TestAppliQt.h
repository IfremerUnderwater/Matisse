#ifndef TESTAPPLIQT_H
#define TESTAPPLIQT_H

#include <QMainWindow>

namespace Ui {
class TestAppliQt;
}

class TestAppliQt : public QMainWindow
{
    Q_OBJECT
    
public:
    explicit TestAppliQt(QWidget *parent = 0);
    ~TestAppliQt();
    
private:
    Ui::TestAppliQt *ui;
};

#endif // TESTAPPLIQT_H

#ifndef USERFORMWIDGET_H
#define USERFORMWIDGET_H

#include <QWidget>
#include <QGraphicsView>

#include "Tools.h"
using namespace MatisseTools;

namespace Ui {
class UserFormWidget;
}


class UserFormWidget : public QWidget
{
    Q_OBJECT
    
public:
    explicit UserFormWidget(QWidget *parent = NULL);
    ~UserFormWidget();

    void init();
    void showUserParameters(Tools *tools = NULL);


    void createCanvas();
    void clear();
    void resetJobForm();
    void loadVectorFile(QString filename = "");
    void loadRasterFile(QString filename = "");
    void setTools(Tools * tools);
    ParametersWidgetSkeleton * parametersWidget();
    
private:
    Ui::UserFormWidget *_ui;
    Tools * _tools;
    ParametersWidgetSkeleton * _parametersWidget;

protected slots:
    void slot_parametersChanged(bool changed);

signals:
    void signal_parametersChanged(bool changed);
};


#endif // USERFORMWIDGET_H

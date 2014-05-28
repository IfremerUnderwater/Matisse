#ifndef USERFORMWIDGET_H
#define USERFORMWIDGET_H

#include <QWidget>
#include <QImage>
#include <QGraphicsView>
#include "Image.h"
#include "Tools.h"
using namespace MatisseTools;
using namespace MatisseCommon;
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
    void showQGisCanvas(bool flag);

    void createCanvas();
    void clear();
    void displayImage(Image *image);
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

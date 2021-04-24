#ifndef EXPERTFORMWIDGET_H
#define EXPERTFORMWIDGET_H

#include <QWidget>
#include <QModelIndex>
#include <QListWidgetItem>
#include <QMessageBox>
#include <QScrollArea>
#include <QAbstractScrollArea>
#include <QResizeEvent>

#include "AssemblyGraphicsScene.h"

#include "SourceWidget.h"
#include "ProcessorWidget.h"
#include "DestinationWidget.h"
#include "PipeWidget.h"
#include "AssemblyGraphicsScene.h"
#include "KeyValueList.h"
#include "ParametersDialog.h"
#include "MatisseParametersManager.h"
#include "ParametersWidgetSkeleton.h"


namespace Ui {
class AssemblyEditor;
}

namespace MatisseServer {

class AssemblyEditor : public QWidget
{
    Q_OBJECT
    
public:
    explicit AssemblyEditor(QWidget *parent = 0);
    ~AssemblyEditor();

    bool loadAssembly(QString assemblyName);
    void resetAssemblyForm();
    AssemblyGraphicsScene * getScene() { return _scene;}
    QGraphicsView * getGraphicsView();

private:
    Ui::AssemblyEditor * _ui;
    AssemblyGraphicsScene * _scene;
    QHash<QString, KeyValueList> _assembliesValues;
//    Server * _server;

    void init();

protected:
    void resizeEvent(QResizeEvent *event);

protected slots:
//    void slot_showParameters(QTreeWidgetItem *item = NULL, int noCol = 0);
    //void slot_parametersValuesModified(bool modified);
    //void slot_changeTabPanel(int panel);

signals:
    //void signal_parametersValuesModified(bool modified);
    //void signal_saveParameters();
    //void signal_selectParameters(bool);
    //void signal_usedParameters(bool);
};

}

#endif // EXPERTFORMWIDGET_H

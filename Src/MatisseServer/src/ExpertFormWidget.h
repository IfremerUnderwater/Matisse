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
#include "Xml.h"
#include "ParametersDialog.h"
#include "Server.h"
#include "MatisseParametersManager.h"
#include "ParametersWidgetSkeleton.h"


namespace Ui {
class ExpertFormWidget;
}

namespace MatisseServer {

class ExpertFormWidget : public QWidget
{
    Q_OBJECT
    
public:
    explicit ExpertFormWidget(QWidget *parent = 0);
    ~ExpertFormWidget();

    // TODO provisoire à supprimer
    void setServer(Server *server);

//    void addProcessorWidget(ProcessorWidget * procWidget);
//    void addSourceWidget(SourceWidget * sourceWidget);
//    void addDestinationWidget(DestinationWidget * destinationWidget);

    //ParametersWidget * getParametersWidget(QString name);
    bool loadAssembly(QString assemblyName);
    bool saveAssembly(QString filename, KeyValueList fields);
    void resetAssemblyForm();
    //void test();
    AssemblyGraphicsScene * getScene() { return _scene;}
    QGraphicsView * getGraphicsView();
    //void showParameters(AssemblyDefinition * assembly);
//    bool saveParameters();
//    bool deleteSelectedParameters();
//    void selectLastUsedParameter();
//    bool selectParametersItem(QString model, QString parameters);

private:
    Ui::ExpertFormWidget * _ui;
    AssemblyGraphicsScene * _scene;
//    QHash<QString, SourceWidget *> _availableSources;
//    QHash<QString, ProcessorWidget *> _availableProcessors;
//    QHash<QString, DestinationWidget *> _availableDestinations;

    //QString _currentAssemblyName;
    //QTreeWidgetItem * _lastUsedParameter;
    //QString _settingsFile;
    //QString _rootXml;
    QHash<QString, KeyValueList> _assembliesValues;
    Server * _server;
    //Xml _xmlTool;
    //Tools * _currentParameters;

    void init();
    //void fillLists();

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

#ifndef EXPERTFORMWIDGET_H
#define EXPERTFORMWIDGET_H

#include <QWidget>
#include <QModelIndex>
#include <QListWidgetItem>
#include <QMessageBox>

#include "AssemblyGraphicsScene.h"

#include "SourceWidget.h"
#include "ProcessorWidget.h"
#include "DestinationWidget.h"
#include "ParametersWidget.h"
#include "PipeWidget.h"
#include "AssemblyGraphicsScene.h"
#include "KeyValueList.h"
#include "Xml.h"
#include "Tools.h"
#include "ParametersDialog.h"
#include "Server.h"

namespace Ui {
class ExpertFormWidget;
}

namespace MatisseServer {
class ExpertFormWidget : public QWidget
{
    Q_OBJECT
    
public:
    explicit ExpertFormWidget(Server *server, QWidget *parent = 0);
    ~ExpertFormWidget();

    void addProcessorWidget(ProcessorWidget * procWidget);
    void addSourceWidget(SourceWidget * sourceWidget);
    void addDestinationWidget(DestinationWidget * destinationWidget);

    ParametersWidget * getParametersWidget(QString name);
    SourceWidget * getSourceWidget(QString name);
    ProcessorWidget * getProcessorWidget(QString name);
    DestinationWidget * getDestinationWidget(QString name);
    bool loadAssembly(QString assemblyName);
    bool saveAssembly(QString filename, KeyValueList fields);
    void resetAssemblyForm();
    void test();
    AssemblyGraphicsScene * getScene() { return _scene;}
    void showParameters(AssemblyDefinition * assembly);
    void showParameters(QString parametersName);
    bool saveParameters();
    void selectLastUsedParameter();
    bool selectParametersItem(QString model, QString parameters);

private:
    Ui::ExpertFormWidget * _ui;
    AssemblyGraphicsScene * _scene;
    QHash<QString, ParametersWidget *> _availableParameters;
    QHash<QString, SourceWidget *> _availableSources;
    QHash<QString, ProcessorWidget *> _availableProcessors;
    QHash<QString, DestinationWidget *> _availableDestinations;

    QString _currentAssemblyName;
    QTreeWidgetItem * _lastUsedParameter;
    QString _settingsFile;
    //QString _rootXml;
    QHash<QString, KeyValueList> _assembliesValues;
    QList<Tools> * _parametersFiles;
    Server * _server;
    //Xml _xmlTool;
    Tools * _currentParameters;

    void init();
    void fillLists();

protected slots:
    void slot_showParameters(QTreeWidgetItem *item = NULL, int noCol = 0);
    void slot_parametersValuesModified(bool modified);
    void slot_changeTabPanel(int panel);

signals:
    void signal_parametersValuesModified(bool modified);
    void signal_saveParameters();
};
}

#endif // EXPERTFORMWIDGET_H

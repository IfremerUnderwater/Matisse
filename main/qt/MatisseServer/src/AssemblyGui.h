#ifndef ASSEMBLYGUI_H
#define ASSEMBLYGUI_H

#include <QMainWindow>
#include <QModelIndex>
#include <QResizeEvent>
#include <QGraphicsScene>
#include <QDateTime>
#include <QFile>
#include <QDir>
#include <QMessageBox>
#include <QPair>
#include <QStandardItemModel>
#include <QProgressBar>

#include <QtDebug>

#include "SourceWidget.h"
#include "ProcessorWidget.h"
#include "DestinationWidget.h"
#include "ParametersWidget.h"
#include "PipeWidget.h"
#include "AssemblyGraphicsScene.h"
#include "KeyValueList.h"
#include "Xml.h"
#include "Server.h"
#include "AssemblyDialog.h"
#include "JobDialog.h"
#include "Tools.h"
#include "UserFormWidget.h"
#include "ExpertFormWidget.h"
#include "Dim2FileReader.h"
#include "StatusMessageWidget.h"

namespace Ui {
class AssemblyGui;
}

namespace MatisseServer {

enum MessageIndicatorLevel {
        IDLE,
        OK,
        WARNING,
        ERROR
     };


class AssemblyGui : public QMainWindow
{
    Q_OBJECT
    
public:
    explicit AssemblyGui(QString settingsFile, QWidget *parent = 0);
    ~AssemblyGui();
    bool setSettingsFile(QString settings);
    bool isShowable();
    void loadAssembliesAndJobsLists();

private:
    Ui::AssemblyGui *_ui;
    bool _userMode;
    Server _server;
    bool _canShow;

    QString _currentJobName;
    QString _settingsFile;
    QString _rootXml;
    QString _dataPath;
    QHash<QString, KeyValueList> _assembliesValues;
    Tools * _parameters;
    bool _beforeSelect;
    QTreeWidgetItem * _lastJobLaunchedItem;

    UserFormWidget * _userFormWidget;
    ExpertFormWidget * _expertFormWidget;
    QBrush _oldBrush;
    QHash<QString, QTreeWidgetItem*> _assembliesItems;

private:
    void init();
    void test();
    bool getAssemblyValues(QString filename, QString  name, bool &valid, KeyValueList & assemblyValues);
    void displayAssembly(QString assemblyName);
    void displayJob(QString jobName);
    void selectJob(QString jobName);
    void showError(QString title, QString message);
    QTreeWidgetItem * addAssemblyInTree(AssemblyDefinition *assembly);
    QTreeWidgetItem * addJobInTree(JobDefinition *job);
    void selectItem(QTreeWidget wid, QString itemText);

    // status bar
    //QProgressBar _statusProgressBar;
    QLabel _statusLed;
    StatusMessageWidget _statusMessageWidget;
    //QComboBox _messagesCombo;
    //QPushButton _messagesResetButton;

    QHash<MessageIndicatorLevel, QString> _messagesIndicators;
    void initStatusBar();
    void showStatusMessage(QString message = "", MessageIndicatorLevel level = IDLE, bool progressOn = false);
    void setActionsStates(QTreeWidgetItem *currentItem = NULL);

    bool _userParameterModified;
    bool _expertValuesModified;

protected slots:
    void slot_showAssembly(QModelIndex index);
    void slot_saveAssembly();
    void slot_saveAsAssembly();
    void slot_deleteAssembly();
    void slot_saveJob();
    void slot_saveAsJob();
    void slot_deleteJob();

    void slot_quit();
    void slot_clearAssembly();
    void slot_swapUserOrExpert();
    void slot_launchJob();
    void slot_stopJob();
    void slot_jobIntermediateResult(QString name, Image *image);
    void slot_jobProcessed(QString name, bool isCancelled);
    void slot_assembliesReload();
    void slot_modifiedParameters(bool changed);
    void slot_selectAssemblyOrJob(QTreeWidgetItem *selectedItem, int column=0);
    void slot_assemblyElementsCount(int count);
    void slot_saveParameters();
    void slot_selectParameters(bool selectedParameters);
    void slot_deleteParameters();
    void slot_usedParameters(bool usedParameters);

};
}

#endif // ASSEMBLYGUI_H

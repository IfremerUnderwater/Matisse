﻿#ifndef ASSEMBLYGUI_H
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
#include <QToolButton>

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
#include "PreferencesDialog.h"
#include "Tools.h"
#include "UserFormWidget.h"
#include "ExpertFormWidget.h"
#include "Dim2FileReader.h"
#include "StatusMessageWidget.h"
#include "MatissePreferences.h"
#include "HomeWidget.h"
#include "MatisseMenu.h"


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
    void loadAssembliesAndJobsLists(bool doExpand=true);
    void loadDefaultStyleSheet();

    void initMainMenu();
    void initStylesheetSelection();
    void initContextMenus();
    void setActionStatesNew();

    SourceWidget * getSourceWidget(QString name);
    ProcessorWidget * getProcessorWidget(QString name);
    DestinationWidget * getDestinationWidget(QString name);

    void initDateTimeDisplay();
    void initPreferences();
    void loadAssemblyParameters(AssemblyDefinition *selectedAssembly);
private:
    Ui::AssemblyGui *_ui;
    bool _isMapView;
    Server _server;
    bool _canShow;

    QString _settingsFile;
    QString _rootXml;
    QString _dataPath;
    Tools * _parameters;
    bool _beforeSelect;
    MatissePreferences* _preferences;
    QTranslator* _toolsTranslator_en;
    QTranslator* _serverTranslator_en;
    QTranslator* _serverTranslator_fr;
    QString _currentLanguage;

    bool _userParameterModified;
    bool _expertValuesModified;

    static const QString PREFERENCES_FILEPATH;

    QTreeWidgetItem * _lastJobLaunchedItem;
    AssemblyDefinition *_newAssembly;
    JobDefinition *_currentJob;
    UserFormWidget * _userFormWidget;
    ExpertFormWidget * _expertFormWidget;
    ParametersWidgetSkeleton * _parametersWidget;
    QLabel* _messagesPicto;

    ApplicationMode _activeApplicationMode;
    QHash<QString, QTreeWidgetItem*> _assembliesItems;
    QHash<QString, KeyValueList*> _assembliesProperties;
    QMap<ApplicationMode, QString> _stylesheetByAppMode;
    QMap<ApplicationMode, QIcon> _stopButtonIconByAppMode;
    QMap<ApplicationMode, QPixmap> _messagePictoByAppMode;
    QMap<MessageIndicatorLevel, QPixmap> _messagePictoByLevel;
    QMap<MessageIndicatorLevel, QIcon> _messagesLevelIcons;
    QIcon _mapVisuModeIcon;
    QIcon _creationVisuModeIcon;
    QToolButton* _visuModeButton;
    QToolButton* _stopButton;
    QIcon _maximizeIcon;
    QIcon _restoreToNormalIcon;
    QToolButton* _maximizeOrRestoreButton;

    QLabel *_activeViewOrModeLabel;
    QLabel *_currentDateTimeLabel;
    QTimer *_dateTimeTimer;

    // status bar
    //QProgressBar _statusProgressBar;
    StatusMessageWidget* _statusMessageWidget;
    //QComboBox _messagesCombo;
    //QPushButton _messagesResetButton;


    QHash<QString, SourceWidget *> _availableSources;
    QHash<QString, ProcessorWidget *> _availableProcessors;
    QHash<QString, DestinationWidget *> _availableDestinations;    

    /* static menu headers */
    MatisseMenu *_fileMenu;
    MatisseMenu *_displayMenu;
    MatisseMenu *_processMenu;
    MatisseMenu *_toolMenu;
    MatisseMenu *_helpMenu;
    QMenu *_mapMenu;

    /* static menu actions */
    QAction* _exportMapViewAct;
    QAction* _exportProjectQGisAct;
    QAction* _closeAct;
    QAction* _dayNightModeAct;
    QAction* _mapToolbarAct;
    QAction* _createAssemblyAct;
    QAction* _saveAssemblyAct;
    QAction* _importAssemblyAct;
    QAction* _exportAssemblyAct;
    QAction* _appConfigAct;
    QAction* _exposureToolAct;
    QAction* _videoToImageToolAct;
    QAction* _checkNetworkRxAct;
    QAction* _loadShapefileAct;
    QAction* _loadRasterAct;
    QAction* _userManualAct;
    QAction* _aboutAct;

    /* assembly context menu */
    QAction* _createJobAct;
    QAction* _importJobAct;
    QAction* _deleteAssemblyAct;
    QAction* _restoreJobAct;
    QAction* _cloneAssemblyAct;
    QAction* _updateAssemblyPropertiesAct;

    /* job context menu */
    QAction* _executeJobAct;
    QAction* _saveJobAct;
    QAction* _cloneJobAct;
    QAction* _exportJobAct;
    QAction* _deleteJobAct;
    QAction* _archiveJobAct;
    QAction* _goToResultsAct;


private:
    void init();
    void test();
    //bool getAssemblyValues(QString filename, QString  name, bool &valid, KeyValueList & assemblyValues);
    void displayAssembly(QString assemblyName);
    void displayJob(QString jobName);
    void selectJob(QString jobName);
    void showError(QString title, QString message);
    QTreeWidgetItem * addAssemblyInTree(AssemblyDefinition *assembly);
    QTreeWidgetItem * addJobInTree(JobDefinition *job);
    void selectItem(QTreeWidget wid, QString itemText);

    void loadStyleSheet(ApplicationMode mode);

    void saveAssemblyAndReload(AssemblyDefinition *assembly);
    void displayAssemblyProperties(AssemblyDefinition *selectedAssembly);

    void initStatusBar();
    void showStatusMessage(QString message = "", MessageIndicatorLevel level = IDLE, bool progressOn = false);
    void setActionsStates(QTreeWidgetItem *currentItem = NULL);

    void initLanguages();
    void updateLanguage(QString language, bool forceRetranslation = FALSE);
    void retranslate();

protected:
    void changeEvent(QEvent *event); // overriding event handler for dynamic translation

protected slots:
    //void slot_showAssembly(QModelIndex index);
    void slot_saveAssembly();
    //void slot_saveAsAssembly();
    void slot_deleteAssembly();
    void slot_newJob();
    void slot_saveJob();
    //void slot_saveAsJob();
    void slot_deleteJob();
    void slot_assemblyContextMenuRequested(const QPoint &pos);

    void slot_maximizeOrRestore();
    void slot_moveWindow(const QPoint &pos);

    void slot_clearAssembly();
    void slot_newAssembly();
    void slot_swapMapOrCreationView();
    void slot_launchJob();
    void slot_stopJob();
    void slot_jobIntermediateResult(QString name, Image *image);
    void slot_jobProcessed(QString name, bool isCancelled);
    void slot_assembliesReload();
    void slot_modifiedParameters(bool changed);
    void slot_selectAssemblyOrJob(QTreeWidgetItem *selectedItem, int column=0);
    void slot_updateTimeDisplay();
    void slot_updatePreferences();

public slots:
    void slot_showApplicationMode(ApplicationMode mode);
    void slot_goHome();

signals:
    void signal_showWelcome();
};
}

#endif // ASSEMBLYGUI_H
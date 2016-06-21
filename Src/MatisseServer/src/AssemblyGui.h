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
#include <QToolButton>
#include <QMenuBar>
#include <QScrollArea>
#include <QScrollBar>
#include <QDesktopServices>
#include <QRegExp>

#include <QtDebug>

#include "ElementWidgetProvider.h"
#include "AssemblyGraphicsScene.h"
#include "KeyValueList.h"
#include "Server.h"
#include "GraphicalCharter.h"
#include "AssemblyDialog.h"
#include "JobDialog.h"
#include "PreferencesDialog.h"
#include "DuplicateDialog.h"
#include "RestoreJobsDialog.h"
#include "NetworkCheckDialog.h"
#include "UserFormWidget.h"
#include "ExpertFormWidget.h"
#include "Dim2FileReader.h"
#include "StatusMessageWidget.h"
#include "MatissePreferences.h"
#include "HomeWidget.h"
#include "MatisseMenu.h"
#include "LiveProcessWheel.h"
#include "AboutDialog.h"
#include "SystemDataManager.h"
#include "ProcessDataManager.h"
#include "PlatformComparisonStatus.h"
#include "StringUtils.h"
#include "MatisseIconFactory.h"
#include "IconizedButtonWrapper.h"
#include "IconizedLabelWrapper.h"
#include "IconizedTreeItemWrapper.h"
#include "MatisseTreeItem.h"
#include "WelcomeDialog.h"

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


enum UserAction {
    SYSTEM_INIT,
    SWAP_VIEW,
    CHANGE_APP_MODE,
    CREATE_ASSEMBLY,
    MODIFY_ASSEMBLY,
    SAVE_ASSEMBLY,
    SAVE_JOB,
    SELECT_ASSEMBLY,
    SELECT_JOB,
    RUN_JOB,
    JOB_COMPLETE,
    STOP_JOB
};

class UserActionContext {
public:
    UserActionContext() :
        _lastActionPerformed(SYSTEM_INIT) {}
    UserAction lastActionPerformed() const { return _lastActionPerformed; }
    void setLastActionPerformed(UserAction lastActionPerformed) {
        qDebug() << "Last action performed : " << lastActionPerformed;
        _lastActionPerformed = lastActionPerformed;
    }

private:
    UserAction _lastActionPerformed;
};

class AssemblyGui : public QMainWindow, ElementWidgetProvider
{
    Q_OBJECT
    
public:
    explicit AssemblyGui(QWidget *parent = 0);
    ~AssemblyGui();
//    bool setSettingsFile(QString settings);
//    bool isShowable();
    void init();
    void loadDefaultStyleSheet();

    virtual SourceWidget * getSourceWidget(QString name);
    virtual ProcessorWidget * getProcessorWidget(QString name);
    virtual DestinationWidget * getDestinationWidget(QString name);

    void applyNewApplicationContext();
    void handleAssemblyModified();
    void checkAndSelectAssembly(QString selectedAssemblyName);
    void checkAndSelectJob(QTreeWidgetItem* selectedItem);
    void resetOngoingProcessIndicators();
    void updatePreferredDatasetParameters();

    void setSystemDataManager(SystemDataManager *systemDataManager);
    void setProcessDataManager(ProcessDataManager *processDataManager);

    void initMapFeatures();
private:
    Ui::AssemblyGui *_ui;
    bool _isMapView;
    Server _server;
//    bool _canShow;
    UserActionContext _context;

    SystemDataManager *_systemDataManager;
    ProcessDataManager *_processDataManager;

//    QString _settingsFile;
    QString _appVersion;

    QString _exportPath;
    QString _importPath;
    QString _archivePath;

    MatissePreferences* _preferences;
    QTranslator* _toolsTranslator_en;
    QTranslator* _toolsTranslator_fr;
    QTranslator* _serverTranslator_en;
    QTranslator* _serverTranslator_fr;
    QString _currentLanguage;

    bool _jobParameterModified;
    bool _isAssemblyModified;
    bool _isAssemblyComplete;

    static const QString PREFERENCES_FILEPATH;
    static const QString ASSEMBLY_EXPORT_PREFIX;
    static const QString JOB_EXPORT_PREFIX;
    static const QString DEFAULT_EXCHANGE_PATH;
    static const QString DEFAULT_ARCHIVE_PATH;
    static const QString DEFAULT_RESULT_PATH;
    static const QString DEFAULT_MOSAIC_PREFIX;

    QTreeWidgetItem * _lastJobLaunchedItem;
    AssemblyDefinition *_newAssembly;
    AssemblyDefinition *_currentAssembly;
    JobDefinition *_currentJob;
    UserFormWidget * _userFormWidget;
    ExpertFormWidget * _expertFormWidget;
    QScrollArea * _parametersDock;
    ParametersWidgetSkeleton * _parametersWidget;
    QLabel* _messagesPicto;

    QTreeWidgetItem *_assemblyVersionPropertyItem;
    QTreeWidgetItem *_assemblyCreationDatePropertyItem;
    QTreeWidgetItem *_assemblyAuthorPropertyItem;
    QTreeWidgetItem *_assemblyCommentPropertyHeaderItem;
    QTreeWidgetItem *_assemblyCommentPropertyItem;
    QLabel *_assemblyCommentPropertyItemText;

    ApplicationMode _activeApplicationMode;
    QHash<QString, QTreeWidgetItem*> _assembliesItems;
    QHash<QString, KeyValueList*> _assembliesProperties;
    QMap<ApplicationMode, QString> _stylesheetByAppMode;
    QMap<ApplicationMode, QString> _wheelColorsByMode;
    QMap<ApplicationMode, QString> _colorsByMode1;
    QMap<ApplicationMode, QString> _colorsByMode2;
    QMap<MessageIndicatorLevel, QString> _colorsByLevel;
    QToolButton* _visuModeButton;
    QToolButton* _stopButton;
    QToolButton* _maximizeOrRestoreButton;
    QToolButton* _closeButton;
    QToolButton* _minimizeButton;
    QToolButton* _homeButton;
    QPushButton* _resetMessagesButton;
    HomeWidget *_homeWidget;
    WelcomeDialog *_welcomeDialog;


    bool _isNightDisplayMode;
    QMap<QString, QString> _currentColorSet;

    QLabel *_activeViewOrModeLabel;
    QLabel *_currentDateTimeLabel;
    QTimer *_dateTimeTimer;
    QLabel *_ongoingProcessInfolabel;
    QLabel *_matisseVersionlabel;
    QProgressBar *_ongoingProcessCompletion;
    LiveProcessWheel *_liveProcessWheel;

    // status bar
    StatusMessageWidget* _statusMessageWidget;

    QHash<QString, SourceWidget *> _availableSources;
    QHash<QString, ProcessorWidget *> _availableProcessors;
    QHash<QString, DestinationWidget *> _availableDestinations;

    MatisseIconFactory *_iconFactory;
    IconizedWidgetWrapper *_maxOrRestoreButtonWrapper;
    IconizedWidgetWrapper *_visuModeButtonWrapper;

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
    void initMainMenu();
    void initIconFactory();
    void initStylesheetSelection();
    void initContextMenus();
    void enableActions();
    void initDateTimeDisplay();
    void initPreferences();
    void initVersionDisplay();
    void loadAssemblyParameters(AssemblyDefinition *selectedAssembly);
    void initParametersWidget();
    void initProcessorWidgets();
    void lookupChildWidgets();
    void initProcessWheelSignalling();
    void initUserActions();
    void initServer();
    void initAssemblyCreationScene();
    void initWelcomeDialog();
    //bool getAssemblyValues(QString filename, QString  name, bool &valid, KeyValueList & assemblyValues);
    void loadAssembliesAndJobsLists(bool doExpand=true);
    void displayAssembly(QString assemblyName);
    void displayJob(QString jobName, bool forceReload = false);
    void selectJob(QString jobName, bool reloadJob = true);
    void selectAssembly(QString assemblyName, bool reloadAssembly = true);
    void showError(QString title, QString message);
    QTreeWidgetItem * addAssemblyInTree(AssemblyDefinition *assembly);
    QTreeWidgetItem * addJobInTree(JobDefinition *job, bool isNewJob = false);
    void selectItem(QTreeWidget wid, QString itemText);

    void loadStyleSheet(ApplicationMode mode);

    void saveAssemblyAndReload(AssemblyDefinition *assembly);
    void displayAssemblyProperties(AssemblyDefinition *selectedAssembly);

    void initStatusBar();
    void showStatusMessage(QString message = "", MessageIndicatorLevel level = IDLE);

    void initLanguages();
    void updateLanguage(QString language, bool forceRetranslation = FALSE);
    void retranslate();
    
    bool loadResultToCartoView(QString resultFile_p);
    
    void doFoldUnfoldParameters(bool doUnfold, bool isExplicitAction = false);

    void freezeJobUserAction(bool freeze_p);

    void handleJobModified();
    QString getActualAssemblyOrJobName(QTreeWidgetItem* currentItem);
    QString getActualNewAssemblyName();
    bool promptAssemblyNotSaved();
    void promptJobNotSaved();

    void deleteAssemblyAndReload(bool promptUser);
    void createExportDir();
    void createImportDir();
    void executeImportWorkflow(bool isJobImportAction = false);
    void executeExportWorkflow(bool isJobExportAction = false);
    void checkArchiveDirCreated();
    bool checkArchivePathChange();

protected:
    void changeEvent(QEvent *event); // overriding event handler for dynamic translation

protected slots:
    void slot_saveAssembly();
    void slot_deleteAssembly();
    void slot_newJob();
    void slot_saveJob();
    void slot_deleteJob();
    void slot_assemblyContextMenuRequested(const QPoint &pos);

    void slot_maximizeOrRestore();
    void slot_quit();
    void slot_moveWindow(const QPoint &pos);

    void slot_clearAssembly();
    void slot_newAssembly();
    void slot_swapMapOrCreationView();
    void slot_launchJob();
    void slot_stopJob();
    void slot_jobShowImageOnMainView(QString name, Image *image);
    void slot_userInformation(QString userText);
    void slot_processCompletion(quint8 percentComplete);
    void slot_showInformationMessage(QString title, QString message);
    void slot_showErrorMessage(QString title, QString message);
    void slot_jobProcessed(QString name, bool isCancelled);
    void slot_assembliesReload();
    void slot_modifiedParameters(bool changed);
    void slot_modifiedAssembly();
    void slot_assemblyComplete(bool isComplete);
    void slot_selectAssemblyOrJob(QTreeWidgetItem *selectedItem, int column=0);
    void slot_updateTimeDisplay();
    void slot_updatePreferences();
    void slot_foldUnfoldParameters();
    void slot_showUserManual();
    void slot_showAboutBox();
    void slot_exportAssembly();
    void slot_importAssembly();
    void slot_exportJob();
    void slot_importJob();
    void slot_goToResult();
    void slot_archiveJob();
    void slot_restoreJobs();
    void slot_duplicateJob();
    void slot_duplicateAssembly();
    void slot_checkNetworkRx();
    void slot_swapDayNightDisplay();
    void slot_exportMapToImage();
    void slot_exportMapToQgisProject();
    void slot_loadShapeFile();
    void slot_loadRasterFile();
    void slot_launchExposureTool();
    void slot_launchVideoToImageTool();

public slots:
    void slot_showApplicationMode(ApplicationMode mode);
    void slot_goHome();
    void slot_show3DFileOnMainView(QString filepath_p);
    void slot_addRasterFileToMap(QString filepath_p);
    void slot_addPolygonToMap(basicproc::Polygon polygon_p, QString polyInsideColor_p, QString layerName_p);
    void slot_addPolylineToMap(basicproc::Polygon polygon_p, QString polyColor_p, QString layerName_p);    
    void slot_addQGisPointsToMap(QList<QgsPoint> pointsList_p, QString pointsColor_p, QString layerName_p);

signals:
    void signal_processRunning();
    void signal_processStopped();
    void signal_processFrozen();
    void signal_updateWheelColors(QString colors);
    void signal_updateColorPalette(QMap<QString,QString>);
    void signal_updateExecutionStatusColor(QString newStatusColorAlias);
    void signal_updateAppModeColors(QString newAppModeColorAlias1, QString newAppModeColorAlias2);
};
}

#endif // ASSEMBLYGUI_H

#include <QStyle>
#include <QDesktopWidget>
#include <QMessageBox>

#include "AssemblyGui.h"
#include "ui_AssemblyGui.h"
#include "MatisseVersionWidget.h"
#include "VisuModeWidget.h"
#include "OngoingProcessWidget.h"
#include "GraphicalCharter.h"
#include "RemoteJobHelper.h"

using namespace MatisseTools;
using namespace MatisseServer;

const QString AssemblyGui::PREFERENCES_FILEPATH = QString("config/MatissePreferences.xml");
const QString AssemblyGui::ASSEMBLY_EXPORT_PREFIX = QString("assembly_export_");
const QString AssemblyGui::JOB_EXPORT_PREFIX = QString("job_export_");
const QString AssemblyGui::JOB_REMOTE_PREFIX = QString("job_remote_");
const QString AssemblyGui::DEFAULT_EXCHANGE_PATH = QString("exchange");
const QString AssemblyGui::DEFAULT_ARCHIVE_PATH = QString("archive");
const QString AssemblyGui::DEFAULT_REMOTE_PATH = QString("remote");
const QString AssemblyGui::DEFAULT_RESULT_PATH = QString("./outReconstruction");
const QString AssemblyGui::DEFAULT_MOSAIC_PREFIX = QString("MyProcess");

AssemblyGui::AssemblyGui(QWidget *parent) :

    QMainWindow(parent),
    _ui(new Ui::AssemblyGui),
    _isNightDisplayMode(false),
    _isMapView(false), // initialized to false (creation) so it will be swapped to true (map) at init
    _currentJob(NULL),
    _lastJobLaunchedItem(NULL),
    _currentAssembly(NULL),
    _newAssembly(NULL),
    _isAssemblyComplete(false),
    _jobParameterModified(false),
    _isAssemblyModified(false),
    _assemblyVersionPropertyItem(NULL),
    _assemblyCreationDatePropertyItem(NULL),
    _assemblyAuthorPropertyItem(NULL),
    _assemblyCommentPropertyHeaderItem(NULL),
    _assemblyCommentPropertyItem(NULL),
    _assemblyCommentPropertyItemText(NULL),
    _systemDataManager(NULL),
    _processDataManager(NULL),
    _exportPath(""),
    _importPath(""),
    _stopButton(NULL),
    _minimizeButton(NULL),
    _maximizeOrRestoreButton(NULL),
    _homeButton(NULL),
    _homeWidget(NULL),
    _visuModeButton(NULL),
    _resetMessagesButton(NULL),
    _maxOrRestoreButtonWrapper(NULL),
    _visuModeButtonWrapper(NULL),
    _server(NULL)
{
    _ui->setupUi(this);
    _server.setJobLauncher(this);

}

AssemblyGui::~AssemblyGui()
{
    delete _iconFactory;
    delete _ui;
    qDebug() << "Delete Gui";
}

void AssemblyGui::initDateTimeDisplay()
{
    slot_updateTimeDisplay();
    _dateTimeTimer = new QTimer(this);
    _dateTimeTimer->setInterval(1000);
    connect(_dateTimeTimer, SIGNAL(timeout()), this, SLOT(slot_updateTimeDisplay()));
    _dateTimeTimer->start();
}

void AssemblyGui::updatePreferredDatasetParameters()
{
    KeyValueList kvl;
    kvl.insert(DATASET_PARAM_OUTPUT_DIR, _preferences->defaultResultPath());
    kvl.insert(DATASET_PARAM_OUTPUT_FILENAME, _preferences->defaultMosaicFilenamePrefix());

    _server.parametersManager()->pushPreferredDatasetParameters(kvl);
}

void AssemblyGui::setSystemDataManager(SystemDataManager *systemDataManager)
{
    _systemDataManager = systemDataManager;
}

void AssemblyGui::setProcessDataManager(ProcessDataManager *processDataManager)
{
    _processDataManager = processDataManager;
}

void AssemblyGui::setRemoteJobHelper(RemoteJobHelper *remoteJobHelper)
{
    _remoteJobHelper = remoteJobHelper;
}

void AssemblyGui::initPreferences()
{
    _preferences = new MatissePreferences();

    QFile prefsFile(PREFERENCES_FILEPATH);

    if (!prefsFile.exists()) {
        // Creating preferences file
        _preferences->setLastUpdate(QDateTime::currentDateTime());
        _preferences->setImportExportPath(DEFAULT_EXCHANGE_PATH);
        _preferences->setArchivePath(DEFAULT_ARCHIVE_PATH);
        _preferences->setDefaultResultPath(DEFAULT_RESULT_PATH);
        _preferences->setDefaultMosaicFilenamePrefix(DEFAULT_MOSAIC_PREFIX);
        _preferences->setProgrammingModeEnabled(false); // By default, programming mode is disabled
        _preferences->setLanguage("FR");

        /* Retrieve local system username as default username for remote execution */
        QString sysUsername = qEnvironmentVariable("USER");
        if (sysUsername.isEmpty()) sysUsername = qEnvironmentVariable("USERNAME");

        _preferences->setRemoteServerAddress(_systemDataManager->getDefaultRemoteServerAddress());
        _preferences->setRemoteUsername(sysUsername);
        _preferences->setRemoteQueueName(_systemDataManager->getDefaultRemoteQueueName());
        _preferences->setRemoteDefaultDataPath(_systemDataManager->getDefaultRemoteDataPath());
        _preferences->setRemoteResultPath(_systemDataManager->getDefaultRemoteResultPath());

        _systemDataManager->writeMatissePreferences(PREFERENCES_FILEPATH, *_preferences);
    } else {
        _systemDataManager->readMatissePreferences(PREFERENCES_FILEPATH, *_preferences);
    }

    updateLanguage(_preferences->language(), true);

    updatePreferredDatasetParameters();
    _archivePath = _preferences->archivePath();
}

void AssemblyGui::initParametersWidget()
{
    _parametersDock = _ui->_SCA_parametersDock;

    _server.parametersManager()->setIconFactory(_iconFactory);
    _server.parametersManager()->generateParametersWidget(_parametersDock);
    _parametersWidget = _server.parametersManager()->parametersWidget();

    _parametersDock->setWidget(_parametersWidget);

    connect(_parametersWidget, SIGNAL(signal_valuesModified(bool)), this, SLOT(slot_modifiedParameters(bool)));
}

void AssemblyGui::initProcessorWidgets()
{
    qDebug() << "Available processors " << _server.getAvailableProcessors().size();
    foreach (Processor * processor, _server.getAvailableProcessors()) {
        qDebug() << "Add processor " << processor->name();
        ProcessorWidget * procWidget = new ProcessorWidget();
        procWidget->setName(processor->name());
        procWidget->setInputsNumber(processor->inNumber());
        procWidget->setOutputsNumber(processor->outNumber());

        QListWidgetItem * newProcItem = new QListWidgetItem(procWidget->getIcon(), procWidget->getName());
        newProcItem->setData(Qt::UserRole, qlonglong(procWidget));
        _availableProcessors.insert(procWidget->getName(), procWidget);
        _ui->_LW_processors->addItem(newProcItem);

        //_expertFormWidget->addProcessorWidget(procWidget);
    }


    qDebug() << "Available ImageProviders " << _server.getAvailableImageProviders().size();
    foreach (ImageProvider * imageProvider, _server.getAvailableImageProviders()) {
        qDebug() << "Add imageProvider " << imageProvider->name();
        SourceWidget * srcWidget = new SourceWidget();
        srcWidget->setName(imageProvider->name());
        srcWidget->setOutputsNumber(imageProvider->outNumber());

        QListWidgetItem * newSrcItem = new QListWidgetItem(srcWidget->getIcon(), srcWidget->getName());
        newSrcItem->setData(Qt::UserRole, qlonglong(srcWidget));
        _availableSources.insert(srcWidget->getName(), srcWidget);
        _ui->_LW_inputs->addItem(newSrcItem);

        //_expertFormWidget->addSourceWidget(srcWidget);
    }

    qDebug() << "Available RasterProviders " << _server.getAvailableRasterProviders().size();
    foreach (RasterProvider * rasterProvider, _server.getAvailableRasterProviders()) {
        qDebug() << "Add rasterProvider " << rasterProvider->name();
        DestinationWidget * destWidget = new DestinationWidget();
        destWidget->setName(rasterProvider->name());
        destWidget->setInputsNumber(rasterProvider->inNumber());

        QListWidgetItem * newDestItem = new QListWidgetItem(destWidget->getIcon(), destWidget->getName());
        newDestItem->setData(Qt::UserRole, qlonglong(destWidget));
        _availableDestinations.insert(destWidget->getName(), destWidget);
        _ui->_LW_outputs->addItem(newDestItem);

        //_expertFormWidget->addDestinationWidget(destWidget);
    }
}

void AssemblyGui::lookupChildWidgets()
{
    // Recherche des boutons d'action et système
    _visuModeButton = findChild<QToolButton*>(QString("_TBU_visuModeSwap"));
    _homeButton = findChild<QToolButton*>(QString("_TB_homeButton"));
    _closeButton = findChild<QToolButton*>(QString("_TBU_closeButton"));
    _maximizeOrRestoreButton = findChild<QToolButton*>(QString("_TBU_maximizeRestoreButton"));
    _minimizeButton = findChild<QToolButton*>(QString("_TBU_minimizeButton"));
    _stopButton = findChild<QToolButton*>(QString("_TBU_stopButton"));
    _resetMessagesButton = findChild<QPushButton*>(QString("_PB_resetMessages"));

    // Recherche des libellés, pictogrammes ou indicateurs
    _activeViewOrModeLabel = findChild<QLabel*>(QString("_LA_activeView"));
    _currentDateTimeLabel = findChild<QLabel*>(QString("_LA_currentDateTime"));
    _ongoingProcessInfolabel = findChild<QLabel*>(QString("_LA_ongoingProcessInfoLabel"));
    _matisseVersionlabel = findChild<QLabel*>(QString("_LAB_matisseVersion"));
    _ongoingProcessCompletion = findChild<QProgressBar*>(QString("_PB_ongoingProcessCompletion"));
    _liveProcessWheel = findChild<LiveProcessWheel*>(QString("_WID_liveProcessWheel"));
    _messagesPicto = findChild<QLabel*>(QString("_LA_messagesPicto"));

    // Recherche autres widgets
    _homeWidget = findChild<HomeWidget*>(QString("homeWidget"));

    // Tabs : object name is set anew explicitely to enable stylesheet ( setObjectName overriden)
    QTabWidget *mapViewTabs = findChild<QTabWidget*>(QString("_TW_mapViewTabs"));
    mapViewTabs->setObjectName("_TW_mapViewTabs");
    QTabWidget *creationViewTabs = findChild<QTabWidget*>(QString("_TW_creationViewTabs"));
    creationViewTabs->setObjectName("_TW_creationViewTabs");

    _userFormWidget = _ui->_WID_mapViewSceneContainer;
    _expertFormWidget = _ui->_WID_creationSceneContainer;
}

void AssemblyGui::initProcessWheelSignalling()
{
    connect(this, SIGNAL(signal_processRunning()), _liveProcessWheel, SLOT(slot_processRunning()));
    connect(this, SIGNAL(signal_processStopped()), _liveProcessWheel, SLOT(slot_processStopped()));
    connect(this, SIGNAL(signal_processFrozen()), _liveProcessWheel, SLOT(slot_processFrozen()));
    connect(this, SIGNAL(signal_updateWheelColors(QString)), _liveProcessWheel, SLOT(slot_updateWheelColors(QString)));
}

void AssemblyGui::initUserActions()
{
    /* Assembly tree actions */
    connect(_ui->_TRW_assemblies, SIGNAL(itemClicked(QTreeWidgetItem*,int)), this, SLOT(slot_selectAssemblyOrJob(QTreeWidgetItem*,int)));
    connect(_ui->_TRW_assemblies, SIGNAL(customContextMenuRequested(QPoint)), this, SLOT(slot_assemblyContextMenuRequested(QPoint)));

    connect(_visuModeButton, SIGNAL(clicked()), this, SLOT(slot_swapMapOrCreationView()));

    // Home action
    connect(_homeWidget, SIGNAL(signal_goHome()), this, SLOT(slot_goHome()));

    // System actions
    connect(_closeButton, SIGNAL(clicked()), this, SLOT(slot_quit()));
    connect(_maximizeOrRestoreButton, SIGNAL(clicked()), this, SLOT(slot_maximizeOrRestore()));
    connect(_minimizeButton, SIGNAL(clicked()), this, SLOT(showMinimized()));
    connect(_ui->_MCB_controllBar, SIGNAL(signal_moveWindow(QPoint)), this, SLOT(slot_moveWindow(QPoint)));

    // Menu actions
    connect(_closeAct, SIGNAL(triggered()), this, SLOT(close()));
    connect(_createAssemblyAct, SIGNAL(triggered()), this, SLOT(slot_newAssembly()));
    connect(_saveAssemblyAct, SIGNAL(triggered()), this, SLOT(slot_saveAssembly()));
    connect(_updateAssemblyPropertiesAct, SIGNAL(triggered()), this, SLOT(slot_saveAssembly()));
    connect(_appConfigAct, SIGNAL(triggered()), this, SLOT(slot_updatePreferences()));
    connect(_userManualAct, SIGNAL(triggered()), this, SLOT(slot_showUserManual()));
    connect(_aboutAct, SIGNAL(triggered()), this, SLOT(slot_showAboutBox()));
    connect(_exportAssemblyAct, SIGNAL(triggered()), this, SLOT(slot_exportAssembly()));
    connect(_importAssemblyAct, SIGNAL(triggered()), this, SLOT(slot_importAssembly()));
    connect(_exportJobAct, SIGNAL(triggered()), this, SLOT(slot_exportJob()));
    connect(_importJobAct, SIGNAL(triggered()), this, SLOT(slot_importJob()));
    connect(_checkNetworkRxAct, SIGNAL(triggered()), this, SLOT(slot_checkNetworkRx()));
    connect(_dayNightModeAct, SIGNAL(triggered()), this, SLOT(slot_swapDayNightDisplay()));
    connect(_mapToolbarAct, SIGNAL(triggered()), _userFormWidget, SLOT(slot_showHideToolbar()));
    connect(_exportMapViewAct, SIGNAL(triggered()), this, SLOT(slot_exportMapToImage()));
    connect(_exportProjectQGisAct, SIGNAL(triggered()), this, SLOT(slot_exportMapToQgisProject()));
    connect(_loadShapefileAct, SIGNAL(triggered()), this, SLOT(slot_loadShapeFile()));
    connect(_loadRasterAct, SIGNAL(triggered()), this, SLOT(slot_loadRasterFile()));
    connect(_preprocessingTool, SIGNAL(triggered()), this, SLOT(slot_launchPreprocessingTool()));
    //connect(_videoToImageToolAct, SIGNAL(triggered()), this, SLOT(slot_launchVideoToImageTool()));

    // Menus contextuels
    connect(_createJobAct, SIGNAL(triggered()), this, SLOT(slot_newJob()));
    connect(_saveJobAct, SIGNAL(triggered()), this, SLOT(slot_saveJob()));
    connect(_deleteAssemblyAct, SIGNAL(triggered()), this, SLOT(slot_deleteAssembly()));
    connect(_restoreJobAct, SIGNAL(triggered()), this, SLOT(slot_restoreJobs()));
    connect(_cloneAssemblyAct, SIGNAL(triggered()), this, SLOT(slot_duplicateAssembly()));

    connect(_executeJobAct, SIGNAL(triggered()), this, SLOT(slot_launchJob()));
    connect(_goToResultsAct, SIGNAL(triggered()), this, SLOT(slot_goToResult()));
    connect(_executeRemoteJobAct, SIGNAL(triggered()), this, SLOT(slot_launchRemoteJob()));
    connect(_uploadDataAct, SIGNAL(triggered()), this, SLOT(slot_uploadJobData()));
    connect(_deleteJobAct, SIGNAL(triggered()), this, SLOT(slot_deleteJob()));
    connect(_archiveJobAct, SIGNAL(triggered()), this, SLOT(slot_archiveJob()));
    connect(_cloneJobAct, SIGNAL(triggered()), this, SLOT(slot_duplicateJob()));

    // Tool button actions
    connect(_stopButton, SIGNAL(clicked()), this, SLOT(slot_stopJob()));
    connect(_ui->_PB_parameterFold, SIGNAL(clicked(bool)), this, SLOT(slot_foldUnfoldParameters()));
}

void AssemblyGui::initServer()
{
    _server.setSystemDataManager(_systemDataManager);
    _server.setProcessDataManager(_processDataManager);
    _server.init();

    connect(&_server, SIGNAL(signal_jobProcessed(QString, bool)), this, SLOT(slot_jobProcessed(QString, bool)));
    connect(&_server, SIGNAL(signal_jobShowImageOnMainView(QString,Image *)), this, SLOT(slot_jobShowImageOnMainView(QString,Image *)));
    connect(&_server, SIGNAL(signal_userInformation(QString)), this, SLOT(slot_userInformation(QString)));
    connect(&_server, SIGNAL(signal_processCompletion(quint8)), this, SLOT(slot_processCompletion(quint8)));
}

void AssemblyGui::initAssemblyCreationScene()
{
    _expertFormWidget->getScene()->setServer(&_server);
    _expertFormWidget->getScene()->setProcessDataManager(_processDataManager);
    _expertFormWidget->getScene()->setElementWidgetProvider(this);
    _expertFormWidget->getScene()->setMessageTarget(this);

    connect(_expertFormWidget->getScene(), SIGNAL(signal_assemblyModified()), this, SLOT(slot_modifiedAssembly()));
    connect(_expertFormWidget->getScene(), SIGNAL(signal_assemblyComplete(bool)), this, SLOT(slot_assemblyComplete(bool)));
}

void AssemblyGui::initWelcomeDialog()
{   
    _welcomeDialog = new WelcomeDialog(this, _iconFactory, _preferences->programmingModeEnabled());
    _welcomeDialog->setObjectName("_D_welcomeDialog");
    _welcomeDialog->setWindowFlags(Qt::Dialog | Qt::FramelessWindowHint);
    _welcomeDialog->show();
}

void AssemblyGui::initMapFeatures()
{
    _userFormWidget->setIconFactory(_iconFactory);
    _userFormWidget->setLayersWidget(_ui->_LW_mapLayers);
    _userFormWidget->initCanvas();
    _userFormWidget->initLayersWidget();
    _userFormWidget->initMapToolBar();

    //    for (int i=0; i < 20 ; i++) {
    //        QListWidgetItem *layer = new QListWidgetItem("Layer " + QString::number(i));
    //        layer->setCheckState(Qt::Checked);
    //        _ui->_LW_mapLayers->addItem(layer);
    //    }


}

void AssemblyGui::dpiScaleWidgets()
{
    GraphicalCharter &graph_charter = GraphicalCharter::instance();
    int dpi_cb_height = graph_charter.dpiScaled(CONTROLLBAR_HEIGHT);

    // Adjust main window size
    this->setMinimumWidth(graph_charter.dpiScaled(MAIN_WINDOW_MIN_WIDTH));
    this->setMinimumHeight(graph_charter.dpiScaled(MAIN_WINDOW_MIN_HEIGHT));

    // Main control bar scaling
    _ui->_MCB_controllBar->setFixedHeight(dpi_cb_height);

    _homeWidget->setFixedWidth(graph_charter.dpiScaled(CB_HOME_BUTTON_WIDTH));
    _homeWidget->setFixedHeight(dpi_cb_height);
    _homeButton->setFixedWidth(graph_charter.dpiScaled(CB_HOME_BUTTON_WIDTH));
    _homeButton->setFixedHeight(dpi_cb_height);
    _homeButton->setIconSize(QSize(graph_charter.dpiScaled(CB_HOME_BUTTON_ICON),graph_charter.dpiScaled(CB_HOME_BUTTON_ICON)));
    _visuModeButton->setIconSize(QSize(graph_charter.dpiScaled(CB_VISU_SWAP_ICON),graph_charter.dpiScaled(CB_HOME_BUTTON_ICON)));

    findChild<MatisseVersionWidget*>(QString("matisseVersionWidget"))->setFixedHeight(dpi_cb_height);
    findChild<MatisseVersionWidget*>(QString("matisseVersionWidget"))->setFixedWidth(graph_charter.dpiScaled(CB_VERSION_WIDTH));
    _matisseVersionlabel->setFixedHeight(dpi_cb_height);

    findChild<VisuModeWidget*>(QString("visuModeWidget"))->setFixedHeight(dpi_cb_height);
    findChild<VisuModeWidget*>(QString("visuModeWidget"))->setFixedWidth(graph_charter.dpiScaled(CB_VISU_INFO_WIDTH));
    _activeViewOrModeLabel->setFixedHeight(graph_charter.dpiScaled(CONTROLLBAR_HEIGHT/2));
    _currentDateTimeLabel->setFixedHeight(1+graph_charter.dpiScaled(CONTROLLBAR_HEIGHT/2)); // +1 is for the rounding

    findChild<QWidget*>(QString("mainMenuWidget"))->setFixedHeight(dpi_cb_height);
    findChild<QMenuBar*>(QString("_MBA_mainMenuBar"))->setFixedHeight(graph_charter.dpiScaled(CONTROLLBAR_HEIGHT/2));

    OngoingProcessWidget* proc_wid = findChild<OngoingProcessWidget*>(QString("ongoingProcessWidget"));
    proc_wid->setFixedHeight(dpi_cb_height);
    proc_wid->setFixedWidth(graph_charter.dpiScaled(CB_ON_PROCESS_WIDTH));
    proc_wid->dpiScale();

    // Right panel
    _ui->_PB_parameterFold->setFixedWidth(dpi_cb_height/3);
    _parametersDock->setFixedWidth(graph_charter.dpiScaled(CB_ON_PROCESS_WIDTH)-_ui->_PB_parameterFold->width());

    // Left panel
    _ui->_SPLIT_leftMenu->setFixedWidth(graph_charter.dpiScaled(CB_HOME_BUTTON_WIDTH+CB_VERSION_WIDTH));

    this->setGeometry(
                QStyle::alignedRect(
                    Qt::LeftToRight,
                    Qt::AlignCenter,
                    this->size(),
                    qApp->desktop()->availableGeometry()
                    )
                );

}

void AssemblyGui::init()
{
    // useless but does not build otherwise -> ugly ..................
//    GraphicalCharter &graph_charter = GraphicalCharter::instance();
//    int a = graph_charter.dpiScaled(0);
//    QString b("");
//    FileUtils::createTempDirectory(b);
//    QString source;
//    QMap<QString,QString> properties;
//    StringUtils::substitutePlaceHolders(source, properties);


    initLanguages();

    initIconFactory();

    initStatusBar();

    lookupChildWidgets();

    initStylesheetSelection();

    // hide current process indicators
    resetOngoingProcessIndicators();

    initMainMenu();

    initContextMenus();

    initProcessWheelSignalling();

    initUserActions();

    initServer();

    initVersionDisplay();

    initParametersWidget();

    initPreferences();

    initRemoteJobHelper();
    
    initAssemblyCreationScene();

    initMapFeatures();

    /* loading processors */
    initProcessorWidgets();

    /* swap to map view */
    slot_swapMapOrCreationView();

    /* notify widgets for initial color palette */
    emit signal_updateColorPalette(_currentColorSet);

    // start current date/time timer
    initDateTimeDisplay();

    initWelcomeDialog();

    dpiScaleWidgets();

}

void AssemblyGui::initRemoteJobHelper()
{
    _remoteJobHelper->setJobLauncher(this);
    _remoteJobHelper->setPreferences(_preferences);
    _remoteJobHelper->init();
}

void AssemblyGui::initIconFactory()
{
    _currentColorSet = FileUtils::readPropertiesFile("lnf/MatisseColorsDay.properties");
    _iconFactory = new MatisseIconFactory(_currentColorSet, "grey", "orange", "orange2");
    connect(this, SIGNAL(signal_updateColorPalette(QMap<QString,QString>)), _iconFactory, SLOT(slot_updateColorPalette(QMap<QString,QString>)));
    connect(this, SIGNAL(signal_updateExecutionStatusColor(QString)), _iconFactory, SLOT(slot_updateExecutionStatusColor(QString)));
    connect(this, SIGNAL(signal_updateAppModeColors(QString,QString)), _iconFactory, SLOT(slot_updateAppModeColors(QString,QString)));
}

void AssemblyGui::initStylesheetSelection()
{
    setStyleSheet("");

    _stylesheetByAppMode.insert(PROGRAMMING, "lnf/MatisseModeProg.css");
    _stylesheetByAppMode.insert(REAL_TIME, "lnf/MatisseModeRt.css");
    _stylesheetByAppMode.insert(DEFERRED_TIME, "lnf/MatisseModeDt.css");
    _stylesheetByAppMode.insert(APP_CONFIG, "lnf/MatisseModeProg.css");

    _wheelColorsByMode.insert(PROGRAMMING, QString("%1-%2-%3").arg("<%color.grey%>").arg("<%color.orange%>").arg("<%color.orange2%>"));
    _wheelColorsByMode.insert(REAL_TIME, QString("%1-%2-%3").arg("<%color.grey%>").arg("<%color.blue%>").arg("<%color.blue2%>"));
    _wheelColorsByMode.insert(DEFERRED_TIME, QString("%1-%2-%3").arg("<%color.grey%>").arg("<%color.mauve%>").arg("<%color.mauve2%>"));
    _wheelColorsByMode.insert(APP_CONFIG, QString("%1-%2-%3").arg("<%color.grey%>").arg("<%color.orange%>").arg("<%color.orange2%>"));

    _colorsByMode1.insert(PROGRAMMING, QString("orange"));
    _colorsByMode1.insert(REAL_TIME, QString("blue"));
    _colorsByMode1.insert(DEFERRED_TIME, QString("mauve"));
    _colorsByMode1.insert(APP_CONFIG, QString("grey"));

    _colorsByMode2.insert(PROGRAMMING, QString("orange2"));
    _colorsByMode2.insert(REAL_TIME, QString("blue2"));
    _colorsByMode2.insert(DEFERRED_TIME, QString("mauve2"));
    _colorsByMode2.insert(APP_CONFIG, QString("grey2"));

    _colorsByLevel.insert(IDLE, QString("grey"));
    _colorsByLevel.insert(OK, QString("level.green"));
    _colorsByLevel.insert(WARNING, QString("level.orange"));
    _colorsByLevel.insert(ERROR, QString("level.red"));

    IconizedWidgetWrapper *closeButtonWrapper = new IconizedButtonWrapper(_closeButton);
    _iconFactory->attachIcon(closeButtonWrapper, "lnf/icons/fermer.svg", false, false);

    IconizedWidgetWrapper *minimizeButtonWrapper = new IconizedButtonWrapper(_minimizeButton);
    _iconFactory->attachIcon(minimizeButtonWrapper, "lnf/icons/reduire.svg", false, false);

    _maxOrRestoreButtonWrapper = new IconizedButtonWrapper(_maximizeOrRestoreButton);
    _iconFactory->attachIcon(_maxOrRestoreButtonWrapper, "lnf/icons/agrandir.svg", false, false);

    IconizedWidgetWrapper *stopButtonWrapper = new IconizedButtonWrapper(_stopButton);
    _iconFactory->attachIcon(stopButtonWrapper, "lnf/icons/Main.svg", false, true);

    IconizedWidgetWrapper *homeButtonWrapper = new IconizedButtonWrapper(_homeButton);
    _iconFactory->attachIcon(homeButtonWrapper, "lnf/icons/Maison.svg", false, false);

    _visuModeButtonWrapper = new IconizedButtonWrapper(_visuModeButton);
    _iconFactory->attachIcon(_visuModeButtonWrapper, "lnf/icons/Cartographie.svg", false, true);

    IconizedWidgetWrapper *resetMessagesButtonWrapper = new IconizedButtonWrapper(_resetMessagesButton);
    _iconFactory->attachIcon(resetMessagesButtonWrapper, "lnf/icons/trash.svg", false, false);

    IconizedWidgetWrapper *messagesPictoWrapper = new IconizedLabelWrapper(_messagesPicto);
    _iconFactory->attachIcon(messagesPictoWrapper, "lnf/icons/Message.svg", true, true);

    /* Day / night signalling for local widgets */
    connect(this, SIGNAL(signal_updateColorPalette(QMap<QString,QString>)), _ui->_PB_parameterFold, SLOT(slot_updateColorPalette(QMap<QString,QString>)));
    connect(this, SIGNAL(signal_updateColorPalette(QMap<QString,QString>)), _userFormWidget, SLOT(slot_updateColorPalette(QMap<QString,QString>)));
}

void AssemblyGui::initMainMenu()
{
    /* Identifying container widget */
    QWidget* menuContainer = findChild<QWidget*>(QString("mainMenuWidget"));

    /* MENU FICHIER */
    _fileMenu = new MatisseMenu(menuContainer);

    _exportMapViewAct = new QAction(this);
    _exportProjectQGisAct = new QAction(this);
    _closeAct = new QAction(this);

    _fileMenu->addAction(_exportMapViewAct);
    _fileMenu->addAction(_exportProjectQGisAct);
    _fileMenu->addSeparator();
    _fileMenu->addAction(_closeAct);

    /* MENU AFFICHAGE */
    _displayMenu = new MatisseMenu(menuContainer);

    _dayNightModeAct = new QAction(this);
    _dayNightModeAct->setCheckable(true);
    _dayNightModeAct->setChecked(false);

    _mapToolbarAct = new QAction(this);
    _mapToolbarAct->setCheckable(true);
    _mapToolbarAct->setChecked(false);

    _displayMenu->addAction(_dayNightModeAct);
    _displayMenu->addSeparator();
    _displayMenu->addAction(_mapToolbarAct);

    /* MENU TRAITEMENTS */
    _processMenu = new MatisseMenu(menuContainer);

    _createAssemblyAct = new QAction(this);
    _saveAssemblyAct = new QAction(this);
    _importAssemblyAct = new QAction(this);
    _exportAssemblyAct = new QAction(this);

    _processMenu->addAction(_createAssemblyAct);
    _processMenu->addAction(_saveAssemblyAct);
    _processMenu->addSeparator();
    _processMenu->addAction(_importAssemblyAct);
    _processMenu->addAction(_exportAssemblyAct);

    /* MENU OUTILS */
    _toolMenu = new MatisseMenu(menuContainer);

    _appConfigAct = new QAction(this);
    _preprocessingTool = new QAction(this);
    //_videoToImageToolAct = new QAction(this);
    _checkNetworkRxAct = new QAction(this);

    /* Sous-menu Cartographie */
    _loadShapefileAct = new QAction(this);
    _loadRasterAct = new QAction(this);

    _toolMenu->addAction(_appConfigAct);
    _toolMenu->addSeparator();
    _toolMenu->addAction(_preprocessingTool);
    //_toolMenu->addAction(_videoToImageToolAct);
    _toolMenu->addSeparator();
    _toolMenu->addAction(_checkNetworkRxAct);

    _mapMenu = new QMenu(_toolMenu);
    _mapMenu->addAction(_loadShapefileAct);
    _mapMenu->addAction(_loadRasterAct);

    _toolMenu->addMenu(_mapMenu);

    /* MENU AIDE */
    _helpMenu = new MatisseMenu(menuContainer);

    _userManualAct = new QAction(this);
    _aboutAct = new QAction(this);

    _helpMenu->addAction(_userManualAct);
    _helpMenu->addAction(_aboutAct);

    QMenuBar* mainMenuBar = findChild<QMenuBar*>(QString("_MBA_mainMenuBar"));
    //mainMenuBar->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);

    mainMenuBar->addMenu(_fileMenu);
    mainMenuBar->addMenu(_displayMenu);
    mainMenuBar->addMenu(_processMenu);
    mainMenuBar->addMenu(_toolMenu);
    mainMenuBar->addMenu(_helpMenu);

}

void AssemblyGui::initContextMenus()
{
    /* Actions pour menu contextuel Traitement */
    _createJobAct = new QAction(this);
    _importJobAct = new QAction(this);
    _cloneAssemblyAct = new QAction(this);
    _deleteAssemblyAct = new QAction(this);
    _restoreJobAct = new QAction(this);
    _updateAssemblyPropertiesAct = new QAction(this);

    /* Actions pour menu contextuel Tâche */
    _executeJobAct = new QAction(this);
    _executeRemoteJobAct = new QAction(this);
    _uploadDataAct = new QAction(this);
    _saveJobAct = new QAction(this);
    _cloneJobAct = new QAction(this);
    _exportJobAct = new QAction(this);
    _deleteJobAct = new QAction(this);
    _archiveJobAct = new QAction(this);
    _goToResultsAct = new QAction(this);

    _ui->_TRW_assemblies->setContextMenuPolicy(Qt::CustomContextMenu);
}

void AssemblyGui::initVersionDisplay()
{
    _appVersion = _systemDataManager->getVersion();
    QStringList versionItems = _appVersion.split(".");

    if (versionItems.size() < 3) {
        qCritical() << QString("Version '%1' defined in XML settings does not conform to the 'X.Y.Z' pattern, default version will be displayed").arg(_appVersion);
        return;
    }

    QString productSerie = versionItems.at(0);
    QString major = versionItems.at(1);
    QString minor = versionItems.at(2);

    QString templateLabel = "MATISSE 3D\nV%1.%2.%3";
    QString fullVersionLabel = templateLabel.arg(productSerie).arg(major).arg(minor);

    _matisseVersionlabel->setText(fullVersionLabel);
}

bool AssemblyGui::loadResultToCartoView(QString resultFile_p, bool remove_previous_scenes)
{
    QFileInfo infoResult(resultFile_p);

    if (!infoResult.exists()) {
        qCritical() << "Result image file not found " << infoResult.absoluteFilePath();
        return false;
    }

    if (_userFormWidget->supportedRasterFormat().contains(infoResult.suffix())){
        qDebug() << "Loading raster layer " << resultFile_p;
        _userFormWidget->loadRasterFile(infoResult.absoluteFilePath());

    }else if (_userFormWidget->supportedVectorFormat().contains(infoResult.suffix())){
        qDebug() << "Loading vector layer " << resultFile_p;
        _userFormWidget->loadShapefile(infoResult.absoluteFilePath());

    }else if (_userFormWidget->supported3DFileFormat().contains(infoResult.suffix())){
        _userFormWidget->load3DFile(infoResult.absoluteFilePath(), remove_previous_scenes);

    }else if (_userFormWidget->supportedImageFormat().contains(infoResult.suffix())){
        qDebug() << "Loading image file " << resultFile_p;
        _userFormWidget->loadImageFile(infoResult.absoluteFilePath());

    }else{
        qDebug() << "Output file format not supported";
        return false;
    }

    return true;

}


void AssemblyGui::loadAssembliesAndJobsLists(bool doExpand)
{
    /* Clearing tree and lists */
    _ui->_TRW_assemblies->clear();
    _assembliesItems.clear();

    /* free all tree item wrappers attached to icons */
    _iconFactory->clearObsoleteIcons();

    // free properties key value lists memory allocation
    QList<KeyValueList*> props = _assembliesProperties.values();
    foreach (KeyValueList* prop, props) {
        delete prop;
    }
    _assembliesProperties.clear();

    /* Load elements */
    _processDataManager->loadAssembliesAndJobs();
    _processDataManager->loadArchivedJobs(_archivePath);

    /* Building assembly tree */
    QStringList validAssemblies = _processDataManager->getAssembliesList();

    foreach(QString assemblyName, validAssemblies) {
        AssemblyDefinition *assembly = _processDataManager->getAssembly(assemblyName);

        // RT or DT assemblies are displayed according to application mode
        if (_activeApplicationMode == REAL_TIME && !assembly->isRealTime()) {
            continue;
        } else if (_activeApplicationMode == DEFERRED_TIME && assembly->isRealTime()) {
            continue;
        }

        addAssemblyInTree(assembly);
    }

    /* Add jobs to assemblies tree */
    QStringList jobsNames = _processDataManager->getJobsNames();
    foreach(QString jobName, jobsNames) {
        JobDefinition * jobDef = _processDataManager->getJob(jobName);

        if (jobDef) {
            addJobInTree(jobDef);
        }
    }

    _ui->_TRW_assemblies->sortItems(0, Qt::AscendingOrder);
    if (doExpand) {
        _ui->_TRW_assemblies->expandAll();
    }

    /* If new assembly is pending, add it to tree */
    /* (This sould be prevented by user action checks */
    if (_newAssembly) {
        addAssemblyInTree(_newAssembly);
    }
}

void AssemblyGui::loadStyleSheet(ApplicationMode mode)
{
    emit signal_updateAppModeColors(_colorsByMode1.value(mode), _colorsByMode2.value(mode));

    // Selecting mode-specific stylesheet
    QString styleSheetForMode = _stylesheetByAppMode.value(mode);
    qDebug() << QString("Stylesheet for mode : %1").arg(styleSheetForMode);

    // Loading stylesheets...

    QFile genericStyleSheet("lnf/Matisse.css");
    QFile modeSpecificStyleSheet(styleSheetForMode);

    if (!genericStyleSheet.exists() || !modeSpecificStyleSheet.exists()) {
        qWarning() << "Stylesheet files not found";
    } else {
        if (!genericStyleSheet.open(QIODevice::ReadOnly)) {
            qWarning() << "Generic stylesheet file could not be opened\n" << genericStyleSheet.error();
        } else {
            QByteArray globalStylesBytes = genericStyleSheet.readAll();
            genericStyleSheet.close();

            if(!modeSpecificStyleSheet.open(QIODevice::ReadOnly)) {
                qWarning() << "Mode specific stylesheet file could not be opened\n" << modeSpecificStyleSheet.error();
            } else {
                // appending specific styles to the global stylesheet
                globalStylesBytes.append(modeSpecificStyleSheet.readAll());
                modeSpecificStyleSheet.close();
            }

            // Substituting stylesheet variables
            QString globalStylesWithVariables(globalStylesBytes);
            QString globalStyles = StringUtils::substitutePlaceHolders(globalStylesWithVariables, _currentColorSet);
            //QString globalStyles = globalStylesWithVariables;

            // Applying stylesheet
            qDebug() << "Applying stylesheet...";
            qApp->setStyleSheet(globalStyles);
        }
    }

    // EXTRA CUSTOMISATIONS

    /* Live process wheel */
    QString wheelColorsTemplate = _wheelColorsByMode.value(mode);
    // resolving day/night colors
    QString wheelColors = StringUtils::substitutePlaceHolders(wheelColorsTemplate, _currentColorSet);
    emit signal_updateWheelColors(wheelColors);

    /* activation des actions selon mode applicatif */
    bool hasExpertFeatures = (mode == PROGRAMMING);
    bool hasRealTimeFeatures = (mode == PROGRAMMING || mode == REAL_TIME);

    _visuModeButton->setVisible(hasExpertFeatures);
    _createAssemblyAct->setVisible(hasExpertFeatures);
    _saveAssemblyAct->setVisible(hasExpertFeatures);

    _checkNetworkRxAct->setVisible(hasRealTimeFeatures);

    // affichage du mode pour TR/TD (sinon affichage de la vue)
    if (mode == REAL_TIME) {
        _activeViewOrModeLabel->setText(tr("Mode : Real time"));
    } else if(mode == DEFERRED_TIME){
        _activeViewOrModeLabel->setText(tr("Mode : Post-processing"));
    }

    // always init application mode with map view
    if (!_isMapView) {
        slot_swapMapOrCreationView();
    } else {
        if (_activeApplicationMode == PROGRAMMING) {
            _activeViewOrModeLabel->setText(tr("View : Cartography"));
        }

        _context.setLastActionPerformed(CHANGE_APP_MODE);
        enableActions();
    }

}


void AssemblyGui::loadDefaultStyleSheet()
{
    loadStyleSheet(PROGRAMMING);
}


bool AssemblyGui::promptAssemblyNotSaved() {

    bool confirmAction = true;

    if (_newAssembly) {

        QString newAssemblyName = getActualNewAssemblyName();

        int userResponse = QMessageBox::question(this, tr("New assembly..."),
                                                 tr("Assembly '%1' not yet saved.\nContinue anyway ?\n(Will delete the current assembly)")
                                                 .arg(newAssemblyName), QMessageBox::Yes | QMessageBox::No, QMessageBox::No);

        if (userResponse==QMessageBox::Yes) {
            deleteAssemblyAndReload(false);
        } else {
            confirmAction = false;
        }

    } else if (_isAssemblyModified) {
        if (!_currentAssembly) {
            qCritical() << "Assembly was modified (parameters or processing chain) but the current assembly is unknown, modifications if any will be lost";

            /* reset modification flag */
            _isAssemblyModified = false;
            return true;
        }

        QString currentAssemblyName = _currentAssembly->name();

        int userResponse = QMessageBox::question(this, tr("Assembly modified..."), tr("Assembly '%1' changed.\nContinue anyway ?").arg(currentAssemblyName), QMessageBox::Yes | QMessageBox::No, QMessageBox::No);

        if (userResponse == QMessageBox::No) {
            confirmAction = false;
        }

    }

    return confirmAction;
}


void AssemblyGui::checkAndSelectAssembly(QString selectedAssemblyName)
{
    if (_newAssembly) {

        QString newAssemblyName = getActualNewAssemblyName();
        selectAssembly(newAssemblyName, false);

        bool confirmAction = promptAssemblyNotSaved();

        if (confirmAction) {
            /* restore selection */
            selectAssembly(selectedAssemblyName);
        }

    } else if (_isAssemblyModified) {
        if (!_currentAssembly) {
            qCritical() << "Assembly parameters were modified but the current assembly is unknown, modifications if any will be lost";

            /* reset modification flag */
            _isAssemblyModified = false;
            return;
        }

        QString currentAssemblyName = _currentAssembly->name();

        /* temporarily cancel selection */
        selectAssembly(currentAssemblyName, false);

        bool confirmAction = promptAssemblyNotSaved();

        if (confirmAction) {
            /* reset modification flag */
            _isAssemblyModified = false;

            /* mark job as saved */
            handleAssemblyModified();

            /* restore selection */
            selectAssembly(selectedAssemblyName);
        }

    } else {
        selectAssembly(selectedAssemblyName);
    }

}


void AssemblyGui::promptJobNotSaved()
{
    if (_jobParameterModified) {
        if (!_currentJob) {
            qCritical() << "Job parameters were modified but the current job is unknown, modifications if any will be lost";

            /* reset modification flag */
            _jobParameterModified = false;

        } else {
            QString currentJobName = _currentJob->name();

            int userResponse = QMessageBox::question(this, tr("Parameters modification..."), tr("Parameters from task '%1' were modified.\nSave mods before going on ?").arg(currentJobName), QMessageBox::Yes | QMessageBox::No, QMessageBox::Yes);

            if (userResponse == QMessageBox::Yes) {
                // save job parameters
                _server.parametersManager()->saveParametersValues(currentJobName, false);
            }

            /* reset modification flag */
            _jobParameterModified = false;

            /* mark job as saved */
            handleJobModified();
        }
    }

}


void AssemblyGui::checkAndSelectJob(QTreeWidgetItem* selectedItem)
{
    if (_jobParameterModified) {
        if (!_currentJob) {
            qCritical() << "Job parameters were modified but the current job is unknown, modifications if any will be lost";

            /* reset modification flag */
            _jobParameterModified = false;

        } else {
            QString currentJobName = _currentJob->name();

            /* temporarily cancel selection */
            selectJob(currentJobName, false);

            /* handle user choice to save job parameters or not */
            promptJobNotSaved();

            /* restore selection */
            _ui->_TRW_assemblies->setCurrentItem(selectedItem);
        }

    }
}

void AssemblyGui::resetOngoingProcessIndicators()
{
    // Reset and hide ongoing process indicator on selection of another assembly / job
    _ongoingProcessInfolabel->setText("");
    _ongoingProcessCompletion->setValue(0);
    _ongoingProcessInfolabel->hide();
    _ongoingProcessCompletion->hide();
    emit signal_processStopped(); // to reset wheel if previous job was frozen
}

void AssemblyGui::slot_selectAssemblyOrJob(QTreeWidgetItem * selectedItem, int column)
{
    Q_UNUSED(column)

    if (_isMapView) {
        resetOngoingProcessIndicators();
    }

    if (!selectedItem->parent()) { // An assembly was selected

        QString selectedAssemblyName = getActualAssemblyOrJobName(selectedItem);

        /* check if same assembly is selected */
        if (_newAssembly) {
            QString newAssemblyName = getActualNewAssemblyName();

            if (selectedAssemblyName == newAssemblyName) {
                qDebug() << "Same assembly selected";
                return;
            }
        } else if (_currentAssembly) {
            if (selectedAssemblyName == _currentAssembly->name()) {
                qDebug() << "Same assembly selected";
                return;
            }
        }

        checkAndSelectAssembly(selectedAssemblyName);
        doFoldUnfoldParameters(!_isMapView);

        if (_isMapView) {
            // disable parameters editing
            _server.parametersManager()->toggleReadOnlyMode(true);

        } else { // Creation view

            _expertFormWidget->getGraphicsView()->setEnabled(true); // activate graphical view
            _saveAssemblyAct->setEnabled(true);
        }

        _context.setLastActionPerformed(SELECT_ASSEMBLY);
        enableActions();

    } else { // A job was selected


        QString selectedJobName = getActualAssemblyOrJobName(selectedItem);

        /* check if same job is selected */
        if (_currentJob) {
            if (selectedJobName == _currentJob->name()) {
                qDebug() << "Same job selected";
                return;
            }
        }

        checkAndSelectJob(selectedItem);
        displayJob(selectedJobName);
        applyNewApplicationContext();
        _server.parametersManager()->toggleReadOnlyMode(false); // enable parameters editing
        doFoldUnfoldParameters(true);

        _exportAssemblyAct->setEnabled(false);

        _context.setLastActionPerformed(SELECT_JOB);
        enableActions();
    }
}

void AssemblyGui::slot_updateTimeDisplay()
{
    QDateTime current = QDateTime::currentDateTime();
    _currentDateTimeLabel->setText(current.toString("dd/MM/yyyy hh:mm"));
}

void AssemblyGui::slot_updatePreferences()
{
    /* hide parameters view to display the dialog on top the dark background */
    bool previousFoldingState = _ui->_PB_parameterFold->getIsUnfolded();
    doFoldUnfoldParameters(false);

    PreferencesDialog dialog(this, _iconFactory, _preferences, false);
    dialog.setFixedHeight(GraphicalCharter::instance().dpiScaled(PD_HEIGHT));
    dialog.setFixedWidth(GraphicalCharter::instance().dpiScaled(PD_WIDTH));
    dialog.setWindowFlags(Qt::Dialog | Qt::FramelessWindowHint);

    /* new preferences are saved if modified */
    if (dialog.exec() == QDialog::Accepted) {
        if (checkArchivePathChange()) {
            _systemDataManager->writeMatissePreferences(PREFERENCES_FILEPATH, *_preferences);
            _processDataManager->loadArchivedJobs(_preferences->archivePath());
            updateLanguage(_preferences->language());
            updatePreferredDatasetParameters();
            // reset import / export path (will be reinitialized at runtime)
            _exportPath = "";
            _importPath = "";

            /* recheck preferences for remote job execution */
            _remoteJobHelper->init();
        }
    }

    /* restore parameters view to initial  state */
    doFoldUnfoldParameters(previousFoldingState);
}

void AssemblyGui::slot_foldUnfoldParameters()
{    
    bool isUnfoldAction = _ui->_PB_parameterFold->getIsUnfolded();
    doFoldUnfoldParameters(isUnfoldAction, true);
}

void AssemblyGui::slot_showUserManual()
{
    QString userManualFileName = "help/MatisseHelp_" + _currentLanguage + ".pdf";

    QFileInfo userManualFile(userManualFileName);

    if (!userManualFile.exists()) {
        QMessageBox::warning(this, tr("User manual"), tr("User manual file '%1' does not exist")
                             .arg(userManualFile.absoluteFilePath()));
        return;
    }

    QUrl url = QUrl::fromLocalFile(userManualFile.absoluteFilePath());
    QDesktopServices::openUrl(url);
}

void AssemblyGui::slot_showAboutBox()
{
    KeyValueList meta;
    meta.append("version", _appVersion);

    AboutDialog about(this, meta);
    about.setWindowFlags(Qt::Dialog | Qt::FramelessWindowHint);
    if (about.exec() != QDialog::Accepted) {
        return;
    }

    //    AboutWebView webDialog(this);
    //    webDialog.exec();
}

void AssemblyGui::createExportDir()
{
    QString importExportDir = _preferences->importExportPath();
    if (importExportDir.isEmpty()) {
        qDebug() << "Import export path not defined in preferences, using default path";
        importExportDir = _systemDataManager->getDataRootDir() + QDir::separator() + DEFAULT_EXCHANGE_PATH;
    }

    _exportPath = importExportDir + QDir::separator() + "export";

    QDir exportDir(_exportPath);
    if (!exportDir.exists()) {
        qDebug() << "Creating export directory " << _exportPath;
        exportDir.mkpath(".");
    }
}


void AssemblyGui::createImportDir()
{
    QString importExportDir = _preferences->importExportPath();
    if (importExportDir.isEmpty()) {
        qDebug() << "Import export path not defined in preferences, using default path";
        importExportDir = _systemDataManager->getDataRootDir() + QDir::separator() + DEFAULT_EXCHANGE_PATH;
    }

    _importPath = importExportDir + QDir::separator() + "import";

    QDir importDir(_importPath);
    if (!importDir.exists()) {
        qDebug() << "Creating import directory " << _importPath;
        importDir.mkpath(".");
    }
}

void AssemblyGui::checkArchiveDirCreated()
{
    QString archiveDirPath = _preferences->archivePath();
    if (archiveDirPath.isEmpty()) {
        qDebug() << "Archive path not defined in preferences, using default path";
        archiveDirPath = _systemDataManager->getDataRootDir() + QDir::separator() + DEFAULT_ARCHIVE_PATH;
    }

    _archivePath = archiveDirPath;

    QDir archiveDir(_archivePath);
    if (!archiveDir.exists()) {
        qDebug() << "Creating archive directory " << _archivePath;
        archiveDir.mkpath(".");
    }
}

void AssemblyGui::checkRemoteDirCreated()
{
    QString remoteRootPath = _systemDataManager->getDataRootDir() + QDir::separator() + DEFAULT_REMOTE_PATH;

    _remoteOutputPath = remoteRootPath + QDir::separator() + "toServer";
    _remoteInputPath = remoteRootPath + QDir::separator() + "fromServer";

    QDir remoteOutputDir(_remoteOutputPath);
    if (!remoteOutputDir.exists()) {
        qDebug() << "Creating remote output directory " << _remoteOutputPath;
        remoteOutputDir.mkpath(".");
    }

    QDir remoteInputDir(_remoteInputPath);
    if (!remoteInputDir.exists()) {
        qDebug() << "Creating remote input directory " << _remoteInputPath;
        remoteInputDir.mkpath(".");
    }
}

void AssemblyGui::slot_exportAssembly()
{
    executeExportWorkflow(false);
}

void AssemblyGui::slot_importAssembly()
{
    executeImportWorkflow();
}

void AssemblyGui::slot_exportJob()
{
    executeExportWorkflow(true);
}

void AssemblyGui::slot_importJob()
{
    executeImportWorkflow(true);
}

void AssemblyGui::slot_goToResult()
{
    QString datasetPath = _server.parametersManager()->getParameterValue(DATASET_PARAM_DATASET_DIR);
    QString resultPath = datasetPath; //+ QDir::separator()  +_server.parametersManager()->getParameterValue(DATASET_PARAM_OUTPUT_DIR);
    QDir resultDir(resultPath);
    if (!resultDir.exists()) {
        QMessageBox::critical(this, tr("Path invalid"), tr("Result path '%1' does not exist.").arg(resultPath));
        return;
    }

    QUrl url = QUrl::fromLocalFile(resultDir.absolutePath());
    QDesktopServices::openUrl(url);
}

void AssemblyGui::slot_archiveJob()
{
    checkArchiveDirCreated();

    if (!_currentJob) {
        // technically inconsistent
        qCritical() << QString("Current job not identified, could not archive");
        return;
    }

    bool archived = _processDataManager->archiveJobFiles(_currentJob->name(), _archivePath);

    if (archived) {
        QMessageBox::information(this, tr("Backup"), tr("Task '%1' has been backed up")
                                 .arg(_currentJob->name()));

        // reload assembly tree
        slot_assembliesReload();

    } else {
        QMessageBox::critical(this, tr("Backup failure"), tr("Task '%1' couldn't be backed up.")
                              .arg(_currentJob->name()));
    }
}

void AssemblyGui::slot_restoreJobs()
{
    if (!_currentAssembly) {
        qCritical() << "Could not identify selected assembly, impossible to restore jobs";
        return;
    }

    QString assemblyName = _currentAssembly->name();
    QStringList archivedJobs = _processDataManager->getAssemblyArchivedJobs(assemblyName);
    QStringList jobsToRestore;

    RestoreJobsDialog dialog(this, assemblyName, archivedJobs, jobsToRestore);
    dialog.setWindowFlags(Qt::Dialog | Qt::FramelessWindowHint);
    if (dialog.exec() != QDialog::Accepted) {
        return;
    }

    if (!jobsToRestore.isEmpty()) {
        bool restored = _processDataManager->restoreArchivedJobs(_archivePath, assemblyName, jobsToRestore);
        if (restored) {
            slot_assembliesReload();
        }
    }

}

void AssemblyGui::slot_duplicateJob()
{
    if (!_currentJob) {
        qCritical() << "The selected job cannot be identified, impossible to duplicate";
        return;
    }

    QString jobName = _currentJob->name();
    QString newJobName;

    QStringList existingJobNames = _processDataManager->getJobsNames();
    QStringList archivedJobNames = _processDataManager->getArchivedJobNames();

    DuplicateDialog dialog(this, jobName, newJobName, false, existingJobNames, archivedJobNames);
    dialog.setWindowFlags(Qt::Dialog | Qt::FramelessWindowHint);
    if (dialog.exec() != QDialog::Accepted) {
        return;
    }

    qDebug() << QString("Duplicating job %1 to %2").arg(jobName, newJobName);
    bool duplicated = _processDataManager->duplicateJob(jobName, newJobName);

    if (duplicated) {
        // reload assembly tree
        slot_assembliesReload();
        QMessageBox::information(this, tr("Task copy"), tr("Task '%1' copied")
                                 .arg(jobName));
    } else {
        QMessageBox::critical(this, tr("Copy failure"), tr("Task '%1' was not copied.")
                              .arg(jobName));
    }
}

void AssemblyGui::slot_duplicateAssembly()
{
    if (!_currentAssembly) {
        qCritical() << "The selected assembly cannot be identified, impossible to duplicate";
        return;
    }

    QString assemblyName = _currentAssembly->name();
    QString newAssemblyName;

    QStringList existingAssemblyNames = _processDataManager->getAssembliesList();

    DuplicateDialog dialog(this, assemblyName, newAssemblyName, true, existingAssemblyNames);
    dialog.setWindowFlags(Qt::Dialog | Qt::FramelessWindowHint);
    if (dialog.exec() != QDialog::Accepted) {
        return;
    }

    qDebug() << QString("Duplicating assembly %1 to %2").arg(assemblyName, newAssemblyName);
    bool duplicated = _processDataManager->duplicateAssembly(assemblyName, newAssemblyName);

    if (duplicated) {
        // reload assembly tree
        slot_assembliesReload();
        QMessageBox::information(this, tr("Processing chain copy"), tr("Processing chain '%1' copied")
                                 .arg(assemblyName));
    } else {
        QMessageBox::critical(this, tr("Processing chain copy failed"), tr("Processing chain '%1' copy failed.")
                              .arg(assemblyName));
    }

}

void AssemblyGui::slot_checkNetworkRx()
{
    NetworkCheckDialog dialog(this);
    dialog.setWindowFlags(Qt::Dialog | Qt::FramelessWindowHint);
    if (dialog.exec() != QDialog::Accepted) {
        return;
    }
}

void AssemblyGui::slot_swapDayNightDisplay()
{
    _isNightDisplayMode = !_isNightDisplayMode;

    QString colorsFilePath;
    if (_isNightDisplayMode) {
        colorsFilePath = "lnf/MatisseColorsNight.properties";
    } else {
        colorsFilePath = "lnf/MatisseColorsDay.properties";
    }

    /* Reload stylesheet with new color set */
    _currentColorSet = FileUtils::readPropertiesFile(colorsFilePath);
    loadStyleSheet(_activeApplicationMode);

    /* notify other widgets */
    emit signal_updateColorPalette(_currentColorSet);
}

void AssemblyGui::slot_exportMapToImage()
{
    if (_exportPath.isEmpty()) {
        createExportDir();
    }

    QString imageFilePath = QFileDialog::getSaveFileName(this, tr("Export current view to image..."), _exportPath, tr("Image file (*.png)"));

    if (imageFilePath.isEmpty()) {
        /* cancel */
        return;
    }

    _userFormWidget->exportMapViewToImage(imageFilePath);

    QMessageBox::information(
                this,
                tr("Export view to image"),
                tr("View has been exported in file %1").arg(imageFilePath));
}

void AssemblyGui::slot_exportMapToQgisProject()
{
    if (_exportPath.isEmpty()) {
        createExportDir();
    }

    QString qgisProjectFilePath = QFileDialog::getSaveFileName(this, tr("Export QGIS project..."), _exportPath, tr("Project file (*.qgs)"));

    if (qgisProjectFilePath.isEmpty()) {
        /* cancel */
        return;
    }

    _userFormWidget->saveQgisProject(qgisProjectFilePath);

    QMessageBox::information(
                this,
                tr("Export QGIS project"),
                tr("Viex exported to QGis project file %1").arg(qgisProjectFilePath));
}

void AssemblyGui::slot_loadShapeFile()
{
    QString shapeFilePath = QFileDialog::getOpenFileName(this, tr("Open shapefile..."), _exportPath, tr("Shapefile (*.shp)"));

    if (shapeFilePath.isEmpty()) {
        /* cancel */
        return;
    }

    _userFormWidget->loadShapefile(shapeFilePath);
}

void AssemblyGui::slot_loadRasterFile()
{
    QString rasterFilePath = QFileDialog::getOpenFileName(this, tr("Open raster..."), _exportPath, tr("Raster file (*.tif *.tiff)"));

    if (rasterFilePath.isEmpty()) {
        /* cancel */
        return;
    }

    _userFormWidget->loadRasterFile(rasterFilePath);
}

void AssemblyGui::slot_launchPreprocessingTool()
{
    QMap<QString, QString> externalTools = _systemDataManager->getExternalTools();
    if (!externalTools.contains("preprocessingTool")) {
        qCritical() << "preprocessingTool tool not defined in settings";
        QMessageBox::critical(this, tr("Incomplete system configuration"), tr("Preprocessing tool not defined in system configuration"));
        return;
    }

    QString toolPath = externalTools.value("preprocessingTool");
    QFileInfo toolPathFile(toolPath);

    if (!toolPathFile.exists()) {
        qCritical() << QString("Could not find exposure tool exe file '%1'").arg(toolPath);
        QMessageBox::critical(this, tr("Tool not found"), tr("Preprocessing tool not found in file '%1'").arg(toolPath));
        return;
    }

    QUrl url = QUrl::fromLocalFile(toolPathFile.absoluteFilePath());
    QDesktopServices::openUrl(url);
}

//void AssemblyGui::slot_launchVideoToImageTool()
//{
//    QMap<QString, QString> externalTools = _systemDataManager->getExternalTools();
//    if (!externalTools.contains("videoToImageTool")) {
//        qCritical() << "Video to image tool not defined in settings";
//        QMessageBox::critical(this, tr("Configuration systeme incomplete"), tr("L'outil de transformation de video en image n'est pas defini dans la configuration systeme"));
//        return;
//    }

//    QString toolPath = externalTools.value("videoToImageTool");
//    QFileInfo toolPathFile(toolPath);

//    if (!toolPathFile.exists()) {
//        qCritical() << QString("Could not find video to image tool exe file '%1'").arg(toolPath);
//        QMessageBox::critical(this, tr("Outil non trouve"), tr("Impossible de trouver le fichier executable de l'outil de transformation de video en image '%1'").arg(toolPath));
//        return;
//    }

//    QUrl url = QUrl::fromLocalFile(toolPathFile.absoluteFilePath());
//    QDesktopServices::openUrl(url);
//}

void AssemblyGui::executeExportWorkflow(bool isJobExportAction, bool isForRemoteExecution) {
    if (isForRemoteExecution && ! isJobExportAction) {
        qCritical() << "Unconsistent call cannot be executed : assembly export for remote execution";
        return;
    }


    /* check unsaved assembly / job */
    bool confirmAction = true;

    if (isJobExportAction) {
        if (_isMapView && _jobParameterModified) {
            promptJobNotSaved();
        }
    } else {
        if (!_isMapView) {
            if (_isAssemblyModified || _newAssembly) {
                confirmAction = promptAssemblyNotSaved();
            }
        }
    }

    if (!confirmAction) {
        return;
    }

    if (isForRemoteExecution) {
        checkRemoteDirCreated();
    } else {
        if (_exportPath.isEmpty()) {
            createExportDir();
        }
    }

    /* common translated labels */
    QString exportFailedTitle = tr("Export failure");
    QString noEntitySelectedMessage = (isJobExportAction) ? tr("No task selected.") : tr("No processing chain were selected.");
    QString exportTitle = (isJobExportAction) ? tr("Taks export") : tr("Export processing chain");
    QString successMessage = (isJobExportAction) ? tr("Task '%1' exported in '%2'") : tr("Processing chain '%1' exported in '%2'");
    QString operationFailedMessage = tr("Operation failed.");

    /* check that an assembly/job is selected */
    bool selected = (isJobExportAction) ? (_currentJob != NULL) : (_currentAssembly != NULL);

    if (!selected) {
        qCritical() << "No assembly/job selected, impossible to export";
        QMessageBox::critical(this, exportFailedTitle, noEntitySelectedMessage);
        return;
    }


    QString entityName = "";
    QString entityFileName = "";
    QString entityPrefix = "";

    if (isJobExportAction) {
        entityName = _currentJob->name();
        entityFileName = _currentJob->filename();
        entityPrefix = (isForRemoteExecution) ? JOB_REMOTE_PREFIX : JOB_EXPORT_PREFIX;

    } else {
        entityName = _currentAssembly->name();
        entityFileName = _currentAssembly->filename();
        entityPrefix = ASSEMBLY_EXPORT_PREFIX;
    }

    qDebug() << QString("Exporting assembly/job '%1'...").arg(entityName);

    QString exportPathRoot = (isForRemoteExecution) ? _remoteOutputPath : _exportPath;

    QString normalizedEntityName = entityFileName;
    normalizedEntityName.chop(4); // remove ".xml" suffix
    QString exportFilename = exportPathRoot + QDir::separator() + entityPrefix + normalizedEntityName + ".zip";

    QStringList fileNames;

    QString assemblyName = "";
    AssemblyDefinition *assembly = NULL;

    if (isJobExportAction) {
        QFileInfo jobFile(_processDataManager->getJobsBasePath() + QDir::separator() + entityFileName);
        if (!jobFile.exists()) {
            qCritical() << QString("File not found for job '%1'. Job could not be exported.").arg(_currentAssembly->name());
            QMessageBox::critical(this, exportFailedTitle, tr("Task file '%1' does not exist.\n").arg(jobFile.absoluteFilePath()).append(operationFailedMessage));
            return;
        }

        fileNames << jobFile.absoluteFilePath();

        QFileInfo jobParametersFile(_processDataManager->getJobParametersFilePath(_currentJob->name()));
        if (!jobParametersFile.exists()) {
            qCritical() << QString("Parameters file not found for job '%1'. Job could not be exported.").arg(_currentJob->name());
            QMessageBox::critical(this, exportFailedTitle, tr("Task parameter file '%1' does not exist.\n").arg(jobParametersFile.absoluteFilePath())
                                  .append(operationFailedMessage));
            return;
        }

        fileNames << jobParametersFile.absoluteFilePath();

        assemblyName = _currentJob->assemblyName();

        assembly = _processDataManager->getAssembly(assemblyName);
        if (!assembly) {
            qCritical() << QString("Assembly '%1' could not be found in local repository").arg(assemblyName);
            QMessageBox::critical(this, exportFailedTitle, tr("Cannot load assembly '%1' parent from task '%2'.\n").arg(assemblyName).arg(entityName)
                                  .append(operationFailedMessage));
            return;
        }
    } else {
        assemblyName = entityName;
        assembly = _currentAssembly;
    }

    QString assemblyFilename = assembly->filename();

    QFileInfo assemblyFile(_processDataManager->getAssembliesPath() + QDir::separator() + assemblyFilename);
    if (!assemblyFile.exists()) {
        qCritical() << QString("File not found for assembly '%1'. Assembly/Job could not be exported.").arg(assemblyName);
        QMessageBox::critical(this, exportFailedTitle, tr("Assembly file' '%1' does not exist.\n").arg(assemblyFile.absoluteFilePath())
                              .append(operationFailedMessage));
        return;
    }

    fileNames << assemblyFile.absoluteFilePath();

    QFileInfo assemblyParametersFile(_processDataManager->getAssembliesParametersPath() + QDir::separator() + assemblyFilename);
    if (assemblyParametersFile.exists()) {
        fileNames << assemblyParametersFile.absoluteFilePath();
    } else {
        qWarning() << QString("Parameters file not found for assembly '%1'. Assembly/Job will be exported without a parameters template file.").arg(assemblyName);
    }

    _systemDataManager->writePlatformSummary();
    _systemDataManager->writePlatformEnvDump();

    fileNames << _systemDataManager->getPlatformSummaryFilePath();
    fileNames << _systemDataManager->getPlatformEnvDumpFilePath();

    //FileUtils::zipFiles(exportFilename, ".", fileNames);
    FileUtils::zipFiles(exportFilename, _systemDataManager->getDataRootDir(), fileNames);

    if (isForRemoteExecution) {
        /* Store path for later use */
        _currentBundleForRemoteExecution = exportFilename;
    } else {
        /* Display export confirmation */
        QMessageBox::information(
                    this,
                    exportTitle,
                    successMessage.arg(entityName).arg(QDir(_exportPath).absolutePath()));
    }
}


void AssemblyGui::executeImportWorkflow(bool isJobImportAction) {
    if (_importPath.isEmpty()) {
        createImportDir();
    }

    /* CHECK UNSAVED ASSEMBLY OR JOB */
    bool confirmAction = true;

    if (!_isMapView) {
        if (_isAssemblyModified || _newAssembly) {
            confirmAction = promptAssemblyNotSaved();
        }
    }

    if (_isMapView && _jobParameterModified) {
        promptJobNotSaved();
    }

    if (!confirmAction) {
        return;
    }

    /* PROMPT USER FOR FILE TO IMPORT */
    QString fileDialogTitle = (isJobImportAction) ? tr("Import a task...") : tr("Import assembly...");
    QString importFilePath = QFileDialog::getOpenFileName(this, fileDialogTitle, _importPath, tr("Export file (*.zip)"));

    if (importFilePath.isEmpty()) {
        // cancelling import operation
        return;
    }

    /* CHECKING SELECTED IMPORT FILE */
    qDebug() << "Checking assembly/job import file...";

    QString dataRootDir = _systemDataManager->getDataRootDir();
    QDir root(dataRootDir);

    QString userDataPath = _systemDataManager->getUserDataPath();
    QString userDataRelativePath = root.relativeFilePath(userDataPath);

    QString assemblyFilePattern = _processDataManager->getAssemblyFilePattern();
    QString assemblyParametersFilePattern = _processDataManager->getAssemblyParametersFilePattern();

    QRegExp assemblyRex(assemblyFilePattern);
    QRegExp assemblyParametersRex(assemblyParametersFilePattern);

    QString jobFilePattern = _processDataManager->getJobFilePattern();
    QString jobParametersFilePattern = _processDataManager->getJobParametersFilePattern();

    QRegExp jobRex(jobFilePattern);
    QRegExp jobParametersRex(jobParametersFilePattern);

    QString remoteAssemblyFilePath = "";
    QString remoteAssemblyParametersFilePath = "";

    QString remoteJobFile = "";
    QString remoteJobParametersFile = "";

    QString platformFilePath = _systemDataManager->getPlatformSummaryFilePath();
    QString remotePlatformFile = root.relativeFilePath(platformFilePath);

    bool foundAssemblyFile = false;
    bool foundAssemblyParametersFile = false;
    bool foundJobFile = false;
    bool foundJobParametersFile = false;
    bool foundPlatformFile = false;

    QString importFailedTitle = tr("Import failed");
    QString importTitle = (isJobImportAction) ? tr("Task import") : tr("Processing chain import");
    QString areYouSureContinueMessage = tr("Sure to continue ?");
    QString areYouSureReplaceMessage = tr("Sure you want to erase ?");
    QString operationFailedMessage = tr("Operation failed.");
    QString operationTemporarilyFailedMessage = tr("Operation temporary failed.\n");
    QString tryAgainLaterMessage = tr("Try again later.");
    QString removeFileManually = tr("Delete manually or try again later.");

    // Parsing archive files to verify that all expected files are present
    QStringList filesList = FileUtils::getZipEntries(importFilePath);

    foreach (QString file, filesList) {
        if (file.startsWith(userDataRelativePath)) {
            if (assemblyRex.exactMatch(file)) {
                qDebug() << "Found assembly file " << file;
                remoteAssemblyFilePath = file;
                foundAssemblyFile = true;
            } else if (assemblyParametersRex.exactMatch(file)) {
                qDebug() << "Found assembly parameters file " << file;
                remoteAssemblyParametersFilePath = file;
                foundAssemblyParametersFile = true;
            }

            if (isJobImportAction) {
                if (jobRex.exactMatch(file)) {
                    qDebug() << "Found job file " << file;
                    remoteJobFile = file;
                    foundJobFile = true;
                } else if (jobParametersRex.exactMatch(file)) {
                    qDebug() << "Found job parameters file " << file;
                    remoteJobParametersFile = file;
                    foundJobParametersFile = true;
                }
            }
        } else {
            if (file == remotePlatformFile) {
                qDebug() << "Found platform file " << remotePlatformFile;
                foundPlatformFile = true;
            }
        }
    }

    if (isJobImportAction) {
        if (!foundJobFile) {
            qCritical() << "Job file could not be found in the export archive " + importFilePath;
            QMessageBox::critical(this, importFailedTitle, tr("Job file could not be found in the export archive."));
            return;
        }

        if (!foundJobParametersFile) {
            qCritical() << "Job parameters file could not be found in the export archive " + importFilePath;
            QMessageBox::critical(this, importFailedTitle, tr("Job parameters file could not be found in the export archive."));
            return;
        }
    }

    if (!foundAssemblyFile) {
        qCritical() << "Assembly file could not be found in the export archive " + importFilePath;
        QMessageBox::critical(this, importFailedTitle, tr("Assembly file could not be found in the export archive."));
        return;
    }

    if (!foundAssemblyParametersFile) {
        qCritical() << "Assembly parameters file could not be found in the export archive " + importFilePath;
        QMessageBox::critical(this, importFailedTitle, tr("Assembly parameters file could not be found in the export archive ."));
        return;
    }

    if (!foundPlatformFile) {
        qCritical() << "Remote platform summary file could not be found in the export archive " + importFilePath;
        QMessageBox::critical(this, importFailedTitle, tr("Remote platform summary file could not be found in the export archive."));
        return;
    }

    /* EXTRACTING ASSEMBLY EXPORT FILE */
    qDebug() << QString("Extracting assembly/job export file '%1'").arg(importFilePath);

    QString tempImportDirPath;
    bool tempDirCreated = FileUtils::createTempDirectory(tempImportDirPath);

    if (!tempDirCreated) {
        qCritical() << QString("Could not create temporary importation dir '%1'.").arg(tempImportDirPath);
        QMessageBox::critical(this, importFailedTitle, operationTemporarilyFailedMessage.append(tryAgainLaterMessage));
        return;
    }

    bool unzipped = FileUtils::unzipFiles(importFilePath, tempImportDirPath);

    if (!unzipped) {
        qCritical() << QString("Could not unzip file '%1' to temporary importation dir '%2'.").arg(importFilePath).arg(tempImportDirPath);
        QMessageBox::critical(this, importFailedTitle, tr("Could not unzip file '%1'.\n")
                              .arg(importFilePath).append(tryAgainLaterMessage));
        return;
    }

    /* COMPARING REMOTE AND LOCAL PLATFORM */
    QString tempRemotePlatformFile = tempImportDirPath + QDir::separator() + remotePlatformFile;

    bool read = _systemDataManager->readRemotePlatformSummary(tempRemotePlatformFile);
    if (!read) {
        QMessageBox::critical(this, importFailedTitle, tr("Import file invalid.\n").append(operationFailedMessage));
        return;
    }

    PlatformComparisonStatus *comparisonStatus = _systemDataManager->compareRemoteAndLocalPlatform();

    if (!comparisonStatus) {
        QMessageBox::critical(this, importFailedTitle, tr("No information available on distant platform.\n").append(operationFailedMessage));
        return;
    }


    if (comparisonStatus->doesMatch()) {
        qDebug() << "Remote and local platforms are identical.";
    } else {

        /* Comparing remote and local Matisse versions */

        if (comparisonStatus->matisseCompared()._versionCompare == DIFFERENT) {
            QMessageBox::critical(this, importFailedTitle, tr("Distant and local Matisse version couldn't be compared.\n").append(operationFailedMessage));
            return;
        } else if (comparisonStatus->matisseCompared()._versionCompare == NEWER) {
            QMessageBox::critical(this, importFailedTitle, tr("Archive comes from newer Matisse version than this one.\n").append(operationFailedMessage));
            return;
        } else if (comparisonStatus->matisseCompared()._versionCompare == OLDER) {
            if (QMessageBox::No == QMessageBox::question(
                        this,
                        importTitle,
                        tr("You are trying to import from an older version of Matisse. Sure to continue ?\n").append(areYouSureContinueMessage),
                        QMessageBox::Yes,
                        QMessageBox::No)) {
                return;
            }
        }
        /* If Matisse versions are identical, execution continues */

        QStringList platformGapMessages = comparisonStatus->getPlatformGapMessages();
        if (!platformGapMessages.isEmpty()) {

            // build stack for platform gap messages
            QString platformGapMessagesStack;

            for (int i = 0; i < platformGapMessages.size() ; i++) {
                QString message = platformGapMessages.at(i);
                platformGapMessagesStack.append(i).append(": ").append(message);
                if (i < platformGapMessages.size() - 1) {
                    platformGapMessagesStack.append("\n");
                }
            }

            if (QMessageBox::No == QMessageBox::question(
                        this,
                        importTitle,
                        tr("You are trying to import a file from a different system version (see below).\n").append(areYouSureContinueMessage)
                        .append("\n").append(platformGapMessagesStack),
                        QMessageBox::Yes,
                        QMessageBox::No)) {
                return;
            }
        }

    }

    /* CHECKING JOB / ASSEMBLY CONSISTENCY */

    QString sourceJobFilePath = "";
    QString jobName = "";
    QString assemblyName;

    if (isJobImportAction) {
        if (!_currentAssembly) {
            /* Impossible case : assembly has to be selected to provide access to job import function,
             * if this happens, the method is called by wrong caller */
            qCritical() << "No assembly selected, impossible to import job";
            return;
        }

        qDebug() << "Checking if imported job is consistent with selected assembly...";

        QString tempUserDataPath = tempImportDirPath + "/" + userDataRelativePath;

        ProcessDataManager tempDataMgr(tempImportDirPath, tempUserDataPath);

        sourceJobFilePath = tempImportDirPath + "/" + remoteJobFile;
        QFileInfo tempJobFileInfo(sourceJobFilePath);
        if (!tempJobFileInfo.exists()) {
            // Technically unconsistent case : something went wront when unzipping
            qCritical() << "Temp job file not found, archive unzipping failed";
            return;
        }

        QString tempJobFileName = tempJobFileInfo.fileName();
        bool read = tempDataMgr.readJobFile(tempJobFileName);

        if (!read) {
            qCritical() << QString("The job file contained in export archive is invalid.");
            QMessageBox::critical(this, importFailedTitle, tr("Task file in archive is invalid.\n")
                                  .append(operationFailedMessage));
            return;
        }

        QStringList tempJobNames = tempDataMgr.getJobsNames();
        if (tempJobNames.isEmpty()) {
            // technically unconsistent case
            qCritical() << "Source job file was read from export archive but was not handled correctly by the data manager";
            return;
        }

        jobName = tempJobNames.at(0);
        JobDefinition *job = tempDataMgr.getJob(jobName);

        if (!job) {
            // technically unconsistent case
            qCritical() << "Source job file was read from export archive but was not handled correctly by the data manager";
            return;
        }

        assemblyName = job->assemblyName();

        if (assemblyName != _currentAssembly->name()) {
            qCritical() << QString("The job file contained in export archive is invalid.").arg(sourceJobFilePath);
            QMessageBox::critical(this, importFailedTitle, tr("The task you're trying to import does not match the selected assembly.\n")
                                  .append(operationFailedMessage));
            return;
        }
    }


    /* CHECKING IF JOB/ASSEMBLY FILES ARE ALREADY PRESENT ON LOCAL PLATFORM */

    QString targetJobFilePath = "";
    QString targetJobParametersFilePath = "";

    QString targetAssemblyFilePath = dataRootDir + "/" + remoteAssemblyFilePath;
    QFile targetAssemblyFile(targetAssemblyFilePath);

    QString targetAssemblyParametersFilePath = dataRootDir + "/" + remoteAssemblyParametersFilePath;
    QFile targetAssemblyParametersFile(targetAssemblyParametersFilePath);

    QString sourceAssemblyFilePath = tempImportDirPath + "/" + remoteAssemblyFilePath;
    QFile sourceAssemblyFile(sourceAssemblyFilePath);

    QString sourceAssemblyParametersFilePath = tempImportDirPath + "/" + remoteAssemblyParametersFilePath;
    QFile sourceAssemblyParametersFile(sourceAssemblyParametersFilePath);

    bool removeTargetJobFile = false;
    bool removeTargetJobParametersFile = false;
    bool removeTargetAssemblyFile = false;
    bool removeTargetAssemblyParametersFile = false;
    bool importAssemblyFiles = false;

    QFile targetJobFile;
    QFile targetJobParametersFile;

    qDebug() << "Checking if job/assembly are already present on local platform...";

    if (isJobImportAction) {
        targetJobFilePath = dataRootDir + "/" + remoteJobFile;
        targetJobFile.setFileName(targetJobFilePath);
        targetJobParametersFilePath = dataRootDir + "/" + remoteJobParametersFile;
        targetJobParametersFile.setFileName(targetJobParametersFilePath);

        if (targetJobFile.exists()) {
            qDebug() << "Job already present on local platform";
            if (QMessageBox::No == QMessageBox::question(
                        this,
                        importTitle,
                        tr("Task %1 already exists.\n")
                        .arg(jobName).append(areYouSureReplaceMessage),
                        QMessageBox::Yes,
                        QMessageBox::No)) {
                return;
            }

            removeTargetJobFile = true;

            if (targetJobParametersFile.exists()) {
                qDebug() << "Job parameters file already present on local platform";
                removeTargetJobParametersFile = true;
            } else {
                /* Technically unconsistent case : the job exists but no job parameters file was found */
                qWarning() << QString("The job '%1' exists on local platform but the job parameters file was not found").arg(jobName);
            }

        } else { // Job file does not exist on local platform

            if (targetJobParametersFile.exists()) {
                /* Technically unconsistent case : the job does not exist on local platform but a job parameters file was found with a corresponding name */
                qWarning() << QString("The job parameters file for job '%1' was found on local platform but job definition file does not exist").arg(jobName);
                removeTargetJobParametersFile = true;
            }
        }

        if (!targetAssemblyFile.exists()) {
            /* Technically unconsistent case : the assembly was selected but its file does not exist */
            qCritical() << QString("The assembly definition file for assembly '%1' was not found on local platform, job import failed").arg(_currentAssembly->name());
            return;
        }

        // check that assembly and assembly parameters file are identical
        if (FileUtils::areFilesIdentical(sourceAssemblyFilePath, targetAssemblyFilePath)) {
            qDebug() << "Remote and local assembly definition files are identical";
        } else {
            qDebug() << "Remote and local assembly definition files are different";
            if (QMessageBox::No == QMessageBox::question(
                        this,
                        importTitle,
                        tr("Processing chain %1 in archive is different from the one on your computer.\n")
                        .arg(_currentAssembly->name()).append(areYouSureReplaceMessage),
                        QMessageBox::Yes,
                        QMessageBox::No)) {
                return;
            }
        }

        removeTargetAssemblyFile = true;
        importAssemblyFiles = true;

        if (targetAssemblyParametersFile.exists()) {
            if (FileUtils::areFilesIdentical(sourceAssemblyParametersFilePath, targetAssemblyParametersFilePath)) {
                qDebug() << "Remote and local assembly parameters files are identical";
            } else {
                qDebug() << "Remote and local assembly parameters files are different";
                if (QMessageBox::No == QMessageBox::question(
                            this,
                            importTitle,
                            tr("Parameters from processing chain %1 in archive is different from the one on your computer.\n")
                            .arg(_currentAssembly->name()).append(areYouSureReplaceMessage),
                            QMessageBox::Yes,
                            QMessageBox::No)) {
                    return;
                }
            }

            removeTargetAssemblyParametersFile = true;

        } else {
            /* Technically unconsistent case : the assembly exists but no assembly parameters file was found */
            qWarning() << QString("The assembly parameters file for assembly '%1' was found on local platform but assembly definition file does not exist").arg(_currentAssembly->name());
            if (QMessageBox::No == QMessageBox::question(
                        this,
                        importTitle,
                        tr("Processing chain %1 exists on this computer but has no parameters file.\n")
                        .arg(_currentAssembly->name()).append(areYouSureReplaceMessage),
                        QMessageBox::Yes,
                        QMessageBox::No)) {
                return;
            }
        }

    } else { // Assembly import action

        importAssemblyFiles = true;

        if (targetAssemblyFile.exists()) {
            qDebug() << "Assembly already present on local platform";
            assemblyName = _processDataManager->getAssemblyNameByFilePath(targetAssemblyFilePath);

            if (QMessageBox::No == QMessageBox::question(
                        this,
                        importTitle,
                        tr("Processing chain %1 already exists on this computer.\n")
                        .arg(assemblyName).append(areYouSureReplaceMessage),
                        QMessageBox::Yes,
                        QMessageBox::No)) {
                return;
            }

            removeTargetAssemblyFile = true;
        }

        if (targetAssemblyParametersFile.exists()) {
            if (targetAssemblyFile.exists()) {
                qDebug() << "Assembly parameters file already present on local platform";
            } else {
                /* Technically unconsistent case */
                qWarning() << QString("The assembly parameters file for assembly '%1' was found on local platform but assembly definition file does not exist").arg(assemblyName);
            }

            removeTargetAssemblyParametersFile = true;
        }

    } // end if (isJobImportAction)


    /* REMOVING FILES TO BE REPLACED */
    qDebug() << "Removing existing assembly/job files...";

    if (removeTargetJobFile) {
        if (!targetJobFile.remove()) {
            qCritical() << QString("Could not remove existing job file '%1'.").arg(targetJobFilePath);
            QMessageBox::critical(this, importFailedTitle, tr("Could not remove existing job file '%1'.\n")
                                  .arg(targetJobFilePath).append(removeFileManually));
            return;
        }
    }

    if (removeTargetJobParametersFile) {
        if (!targetJobParametersFile.remove()) {
            qCritical() << QString("Could not remove existing job parameters file '%1'.").arg(targetJobParametersFilePath);
            QMessageBox::critical(this, importFailedTitle, tr("Could not remove existing job parameters file '%1'.\n")
                                  .arg(targetJobParametersFilePath).append(removeFileManually));
            return;
        }
    }

    if (removeTargetAssemblyFile) {
        if (!targetAssemblyFile.remove()) {
            qCritical() << QString("Could not remove existing assembly file '%1'.").arg(targetAssemblyFilePath);
            QMessageBox::critical(this, importFailedTitle, tr("Could not remove existing assembly file '%1'.\n")
                                  .arg(targetAssemblyFilePath).append(removeFileManually));
            return;
        }
    }

    if (removeTargetAssemblyParametersFile) {
        if (!targetAssemblyParametersFile.remove()) {
            qCritical() << QString("Could not remove existing assembly parameters file '%1'.").arg(targetAssemblyParametersFilePath);
            QMessageBox::critical(this, importFailedTitle, tr("Could not remove existing assembly parameters file '%1'.\n")
                                  .arg(targetAssemblyParametersFilePath).append(removeFileManually));
            return;
        }
    }


    /* PROCEEDING WITH IMPORT */
    qDebug() << "Proceeding with assembly/job import...";
    if (isJobImportAction) {
        QFile sourceJobFile(sourceJobFilePath);

        qDebug() << "Copying job file...";
        if (!sourceJobFile.copy(targetJobFilePath)) {
            qCritical() << QString("Could not copy job file '%1' to '%2'.").arg(sourceJobFilePath).arg(targetJobFilePath);
            QMessageBox::critical(this, importFailedTitle, tr("Could not copy job file"));
            return;
        } else {
            qDebug() << "... done";
        }

        QString sourceJobParametersFilePath = tempImportDirPath + "/" + remoteJobParametersFile;
        QFile sourceJobParametersFile(sourceJobParametersFilePath);

        qDebug() << "Copying job parameters file...";
        if (!sourceJobParametersFile.copy(targetJobParametersFilePath)) {
            qCritical() << QString("Could not copy job parameters file '%1' to '%2'.").arg(sourceJobFilePath).arg(targetJobFilePath);
            QMessageBox::critical(this, importFailedTitle, tr("Could not copy job parameters file"));
            return;
        } else {
            qDebug() << "... done";
        }
    }

    // Assembly files are copied in case of assembly import, or job import with user confirmation for replacement
    if (importAssemblyFiles) {
        qDebug() << "Copying assembly file...";
        if (!sourceAssemblyFile.copy(targetAssemblyFilePath)) {
            qCritical() << QString("Could not copy assembly file '%1' to '%2'.").arg(sourceAssemblyFilePath).arg(targetAssemblyFilePath);
            QMessageBox::critical(this, importFailedTitle, tr("Could not copy assembly file"));
            return;
        } else {
            qDebug() << "... done";
        }

        qDebug() << "Copying assembly parameters file...";
        if (!sourceAssemblyParametersFile.copy(targetAssemblyParametersFilePath)) {
            qCritical() << QString("Could not copy assembly parameters file '%1' to '%2'.").arg(sourceAssemblyFilePath).arg(targetAssemblyFilePath);
            QMessageBox::critical(this, importFailedTitle, tr("Could not copy assembly parameters file"));
            return;
        } else {
            qDebug() << "... done";
        }
    }

    /* Reload assembly tree */
    slot_assembliesReload();

    /* Displaying success message */
    QString successMessage = "";

    if (isJobImportAction) {
        if (jobName.isEmpty()) {
            jobName = _processDataManager->getJobNameByFilePath(sourceJobFilePath);
        }

        successMessage = tr("Task '%1' successfully imported.").arg(jobName);
    } else {
        if (assemblyName.isEmpty()) {
            assemblyName = _processDataManager->getAssemblyNameByFilePath(sourceAssemblyFilePath);
        }

        successMessage = tr("Processing chain '%1' successfully imported.").arg(assemblyName);
    }

    QMessageBox::information(
                this,
                importTitle,
                successMessage);
}


void AssemblyGui::doFoldUnfoldParameters(bool doUnfold, bool isExplicitAction)
{
    if (doUnfold) {
        qDebug() << "Unfolding parameters";
        _parametersDock->show();
        _ui->_PB_parameterFold->setToolTip(tr("Fold parameters window"));
    } else {
        qDebug() << "Folding parameters";
        _parametersDock->hide();
        _ui->_PB_parameterFold->setToolTip(tr("Unfold parameters window"));
    }

    if (!isExplicitAction) {
        // force button state change and repaint if state change is not explicitely
        // triggered by the fold/unfold button
        _ui->_PB_parameterFold->setIsUnfolded(doUnfold);
    }
}


void AssemblyGui::freezeJobUserAction(bool freeze_p)
{
    if(freeze_p){
        _ui->_TRW_assemblies->setEnabled(false);
        _ui->_MCB_controllBar->setSwitchModeButtonEnable(false);
    }else{
        _ui->_TRW_assemblies->setEnabled(true);
        _ui->_MCB_controllBar->setSwitchModeButtonEnable(true);
    }
}


bool AssemblyGui::checkArchivePathChange()
{
    if (!_archivePath.isEmpty()) {
        QString newArchivePath = _preferences->archivePath();
        if (newArchivePath != _archivePath) {
            qDebug() << "Archive path has changed, checking if jobs were already archived...";
            bool hasArchivedFiles = false;

            QDir archiveDir(_archivePath);
            if (archiveDir.exists()) {
                QStringList entries = archiveDir.entryList(QDir::Files);
                if (!entries.isEmpty()) {
                    foreach (QString entry, entries) {
                        if (entry.endsWith(".xml")) {
                            hasArchivedFiles = true;
                            break;
                        }
                    }
                }
            }

            if (hasArchivedFiles) {
                QString fullArchivePath = archiveDir.absolutePath();
                QMessageBox::critical(this, tr("Archive folder"), tr("Archive folder '%1' already contains task(s) and cannot be modified.")
                                      .arg(fullArchivePath));
                /* restoring previous archive path */
                _preferences->setArchivePath(_archivePath);
                return false;
            }
        }
    }

    return true;
}

void AssemblyGui::slot_showApplicationMode(ApplicationMode mode)
{
    qDebug() << "Start application in mode " << mode;
    _activeApplicationMode = mode;
    loadStyleSheet(mode);

    if (mode == APP_CONFIG) {
        PreferencesDialog dialog(this, _iconFactory, _preferences);
        dialog.setWindowFlags(Qt::Dialog | Qt::FramelessWindowHint);
        hide();
        /* If preferences are validated, new preferences are persisted */
        if (dialog.exec() == QDialog::Accepted) {
            if (checkArchivePathChange()) {
                _systemDataManager->writeMatissePreferences(PREFERENCES_FILEPATH, *_preferences);
                _processDataManager->loadArchivedJobs(_preferences->archivePath());
                updateLanguage(_preferences->language());
                updatePreferredDatasetParameters();
                // reset import / export path (will be reinitialized at runtime)
                _exportPath = "";
                _importPath = "";
            }
        }

        // retour à l'écran d'accueil
        slot_goHome();
    } else {
        slot_clearAssembly();
        _ui->_TRW_assemblies->clearSelection();
        _ui->_TRW_assemblyInfo->clear();
        // On recharge pour n'afficher que les traitements utilisables pour le mode
        slot_assembliesReload();

        // Reset and hide ongoing process indicators
        resetOngoingProcessIndicators();
        emit signal_processStopped(); // to reset wheel if previous job was frozen

        applyNewApplicationContext();
        doFoldUnfoldParameters(false);

        show();

        // hack with QT 5.10......
        QSize sz = this->size();
        sz.setWidth((sz.width()+1));
        resize(sz);
        sz.setWidth((sz.width()-1));
        resize(sz);
    }
}

void AssemblyGui::slot_goHome()
{
    bool confirmAction = true;

    if (!_isMapView) {
        if (_isAssemblyModified || _newAssembly) {
            confirmAction = promptAssemblyNotSaved();
        }
    }

    if (_isMapView && _jobParameterModified) {
        promptJobNotSaved();
    }

    if (confirmAction) {
        _newAssembly = NULL;
        _currentAssembly = NULL;
        _currentJob = NULL;

        hide();
        _welcomeDialog->enableProgrammingMode(_preferences->programmingModeEnabled());
        _welcomeDialog->show();
    }
}

void AssemblyGui::slot_show3DFileOnMainView(QString filepath_p)
{
    _userFormWidget->load3DFile(filepath_p);
}

void AssemblyGui::slot_addRasterFileToMap(QString filepath_p)
{
    _userFormWidget->loadRasterFile(filepath_p);
}

void AssemblyGui::slot_addPolygonToMap(basicproc::Polygon polygon_p, QString polyInsideColor_p, QString layerName_p)
{
    _userFormWidget->addPolygonToMap(polygon_p, polyInsideColor_p, layerName_p);
}

void AssemblyGui::slot_addPolylineToMap(basicproc::Polygon polygon_p, QString polyColor_p, QString layerName_p)
{
    _userFormWidget->addPolylineToMap(polygon_p, polyColor_p, layerName_p);
}

void AssemblyGui::slot_sshTransferFinished()
{
    _remoteJobHelper->scheduleJob();
}

//void AssemblyGui::slot_addQGisPointsToMap(QList<QgsPoint> pointsList_p, QString pointsColor_p, QString layerName_p)
//{
//    _userFormWidget->addQGisPointsToMap(pointsList_p, pointsColor_p, layerName_p);

//}

void AssemblyGui::saveAssemblyAndReload(AssemblyDefinition *assembly)
{
    /* Save assembly and associated parameters */
    QString assemblyName = assembly->name();
    _expertFormWidget->getScene()->updateAssembly(assembly);
    _processDataManager->writeAssemblyFile(assembly, true);
    _server.parametersManager()->saveParametersValues(assemblyName, true);

    if (assembly == _newAssembly) {
        _newAssembly->deleteLater();
        _newAssembly = NULL;
    }

    _isAssemblyModified = false;

    slot_assembliesReload();

    // re-sélection de l'assemblage enregistré
    QTreeWidgetItem *item = _assembliesItems.value(assemblyName);
    if (item) {
        _ui->_TRW_assemblies->setCurrentItem(item);
        item->setSelected(true);
        // affichage de l'assemblage dans la vue graphique
        displayAssembly(assemblyName);
    } else {
        qWarning() << "Saved assembly is not found in reloaded tree : " << assemblyName;

        // on désactive la vue graphique
        _expertFormWidget->getScene()->reset();
        _expertFormWidget->getGraphicsView()->setEnabled(false);
    }
}

void AssemblyGui::slot_saveAssembly()
{
    if (_newAssembly) {

        qDebug() << "Saving new assembly " << _newAssembly->name();

        // WORFLOW OPTION DO NOT REMOVE
        // Review assembly properties before saving

        //        QString assemblyName = getActualNewAssemblyName();

        //        KeyValueList *props = _assembliesProperties.value(assemblyName);
        //        AssemblyDialog dialog(this, assemblyName, *props, false, true);
        //        dialog.setWindowFlags(Qt::Dialog | Qt::FramelessWindowHint);
        //        if (dialog.exec() != QDialog::Accepted) {
        //            return;
        //        }

        //        _newAssembly->setName(assemblyName);
        //        _newAssembly->setAuthor(props->getValue("Author"));
        //        _newAssembly->setVersion(props->getValue("Version"));
        //        _newAssembly->setUsable(props->getValue("Valid").toInt());
        //        _newAssembly->setComment(props->getValue("Comments"));

        // END WORKFLOW OPTION

        QString assemblyName = getActualNewAssemblyName();
        _newAssembly->setName(assemblyName);
        saveAssemblyAndReload(_newAssembly);
        _context.setLastActionPerformed(SAVE_ASSEMBLY);
        enableActions();

    } else { // Save existing assembly


        QTreeWidgetItem *selectedItem = _ui->_TRW_assemblies->currentItem();
        if (!selectedItem) {
            qCritical() << "Selected item unknown. Assembly cannot be saved";
            return;
        }

        QString assemblyName = getActualAssemblyOrJobName(selectedItem);

        qDebug() << "Saving assembly " << selectedItem;

        AssemblyDefinition *assembly = _processDataManager->getAssembly(assemblyName);
        if (assembly) {
            // mise à jour des propriétés
            KeyValueList *props = _assembliesProperties.value(assemblyName);

            AssemblyDialog dialog(this, assemblyName, *props, false, false);
            dialog.setWindowFlags(Qt::Dialog | Qt::FramelessWindowHint);
            if (dialog.exec() != QDialog::Accepted) {
                return;
            }

            assembly->setVersion(props->getValue("Version"));
            assembly->setUsable(props->getValue("Valid").toInt());
            assembly->setComment(props->getValue("Comments"));

            saveAssemblyAndReload(assembly);

            _context.setLastActionPerformed(SAVE_ASSEMBLY);
            enableActions();
        } else {
            qWarning() << "Assembly not found in session repository : " << assemblyName;

            // on désactive la vue graphique
            _expertFormWidget->getScene()->reset();
            _expertFormWidget->getGraphicsView()->setEnabled(false);
        }

    }

    //    _createAssemblyAct->setEnabled(true);

    //    // activer / désactiver menus contextuels
    //    _cloneAssemblyAct->setVisible(true);
    //    _updateAssemblyPropertiesAct->setVisible(true);
}



void AssemblyGui::displayAssemblyProperties(AssemblyDefinition *selectedAssembly)
{
    GraphicalCharter &graph_chart = GraphicalCharter::instance();

    _ui->_TRW_assemblyInfo->clear();

    /* allocate items */
    _assemblyVersionPropertyItem = new QTreeWidgetItem();
    _assemblyCreationDatePropertyItem = new QTreeWidgetItem();
    _assemblyAuthorPropertyItem = new QTreeWidgetItem();
    _assemblyCommentPropertyHeaderItem = new QTreeWidgetItem();
    _assemblyCommentPropertyItem = new QTreeWidgetItem();

    _assemblyCommentPropertyItemText = new QLabel(selectedAssembly->comment());
    _assemblyCommentPropertyItemText->setObjectName("_LA_assemblyCommentProperty");
    _assemblyCommentPropertyItemText->setWordWrap(true);

    /* assign text values */
    _assemblyVersionPropertyItem->setText(0, tr("Version:"));
    _assemblyVersionPropertyItem->setText(1, selectedAssembly->version());

    _assemblyCreationDatePropertyItem->setText(0, tr("Creation date:"));
    _assemblyCreationDatePropertyItem->setText(1, selectedAssembly->date());

    _assemblyAuthorPropertyItem->setText(0, tr("Author:"));
    _assemblyAuthorPropertyItem->setText(1, selectedAssembly->author());

    _assemblyCommentPropertyHeaderItem->setText(0, tr("Comment:"));

    _assemblyCommentPropertyItemText->setText(selectedAssembly->comment());
    _assemblyCommentPropertyItemText->setToolTip(selectedAssembly->comment()); // to ensure full comment display

    _ui->_TRW_assemblyInfo->addTopLevelItem(_assemblyVersionPropertyItem);
    _ui->_TRW_assemblyInfo->addTopLevelItem(_assemblyCreationDatePropertyItem);
    _ui->_TRW_assemblyInfo->addTopLevelItem(_assemblyAuthorPropertyItem);
    _ui->_TRW_assemblyInfo->addTopLevelItem(_assemblyCommentPropertyHeaderItem);
    _ui->_TRW_assemblyInfo->addTopLevelItem(_assemblyCommentPropertyItem);

    _ui->_TRW_assemblyInfo->header()->resizeSection(0, graph_chart.dpiScaled(ASSEMBLY_PROPS_LABEL_WIDTH));
    _assemblyCommentPropertyHeaderItem->setFirstColumnSpanned(true);
    _assemblyCommentPropertyItem->setFirstColumnSpanned(true);
    _ui->_TRW_assemblyInfo->setItemWidget(_assemblyCommentPropertyItem, 0, _assemblyCommentPropertyItemText);
}

void AssemblyGui::loadAssemblyParameters(AssemblyDefinition *selectedAssembly)
{
    // reinitialisation des parametres avec leurs valeurs par defaut
    _server.parametersManager()->restoreParametersDefaultValues();

    // chargement des parametres attendus par les modules de l'assemblage
    _server.parametersManager()->clearExpectedParameters();
    _server.addParametersForImageProvider(selectedAssembly->sourceDefinition()->name());
    foreach (ProcessorDefinition* processor, selectedAssembly->processorDefs()) {
        _server.addParametersForProcessor(processor->name());
    }
    _server.addParametersForRasterProvider(selectedAssembly->destinationDefinition()->name());

    // chargement des valeurs de paramètres du template de l'assemblage
    _server.parametersManager()->loadParameters(selectedAssembly->name(), true);
}

void AssemblyGui::displayAssembly(QString assemblyName) {

    _ui->_TRW_assemblyInfo->clear();
    if(_userFormWidget) {
        _userFormWidget->clear();
    }

    _currentJob = NULL;

    AssemblyDefinition *selectedAssembly = _processDataManager->getAssembly(assemblyName);

    if (!selectedAssembly) {
        qCritical() << QString("Assembly '%1' not found").arg(assemblyName);
        return;
    }

    _currentAssembly = selectedAssembly;

    if (_isMapView) {

        loadAssemblyParameters(selectedAssembly);
        displayAssemblyProperties(selectedAssembly);

    } else {

        _server.parametersManager()->restoreParametersDefaultValues();
        _expertFormWidget->loadAssembly(assemblyName);
        _server.parametersManager()->loadParameters(assemblyName, true);

    }
}

// TODO Verif modif chrisar & ifremer simultanee (bug reload)
void AssemblyGui::displayJob(QString jobName, bool forceReload)
{
    GraphicalCharter &graph_chart = GraphicalCharter::instance();

    _ui->_TRW_assemblyInfo->clear();
    if(_userFormWidget) {
        _userFormWidget->clear();
    }

    _currentAssembly = NULL;

    bool alreadySelected = false;

    if (_currentJob) {
        if (jobName == _currentJob->name()) {
            alreadySelected = true;
        }
    }

    if (alreadySelected && !forceReload) {
        qDebug() << QString("Job '%1' already selected").arg(jobName);
        return;
    }

    AssemblyDefinition *selectedAssembly = NULL;

    if (!alreadySelected) {
        _currentJob = _processDataManager->getJob(jobName);

        if (!_currentJob) {
            qCritical() << QString("No job definition file found for selected job '%1'").arg(jobName);
            return;
        }
    }

    QString assemblyName = _currentJob->assemblyName();
    selectedAssembly = _processDataManager->getAssembly(assemblyName);

    if (!selectedAssembly) {
        qCritical() << QString("Assembly file not found for assembly name '%1' owning job 'not found'%2'").arg(assemblyName).arg(jobName);
        return;
    }

    loadAssemblyParameters(selectedAssembly);
    _server.parametersManager()->loadParameters(jobName, false);

    /* DISPLAY JOB PROPERTIES AND RESULT IMAGES */
    QString comments = _currentJob->comment();

    QTreeWidgetItem *jobCommentLabelItem = NULL;
    QTreeWidgetItem *jobCommentItem = NULL;
    QLabel *jobCommentItemText = NULL;

    if (comments != "") {
        jobCommentLabelItem = new QTreeWidgetItem();
        jobCommentLabelItem->setText(0, tr("Comment:"));
        _ui->_TRW_assemblyInfo->addTopLevelItem(jobCommentLabelItem);

        jobCommentItem = new QTreeWidgetItem();
        _ui->_TRW_assemblyInfo->addTopLevelItem(jobCommentItem);

        jobCommentItemText = new QLabel();
        jobCommentItemText->setObjectName("_LA_jobCommentProperty");
        jobCommentItemText->setText(comments);
        jobCommentItemText->setToolTip(comments);
        jobCommentItemText->setWordWrap(true);
    }

    QTreeWidgetItem *jobResultLabelItem = NULL;
    QTreeWidgetItem **jobResultImageItems = NULL;
    int nbOfImageFiles = 0;

    if (_currentJob->executionDefinition() && _currentJob->executionDefinition()->executed()) {
        QTreeWidgetItem *jobExecutionDateItem = new QTreeWidgetItem();
        jobExecutionDateItem->setText(0, tr("Execution date:"));
        QString executionDateStr = _currentJob->executionDefinition()->executionDate().toString(tr("dd/MM/yyyy HH:mm"));
        jobExecutionDateItem->setText(1, executionDateStr);
        _ui->_TRW_assemblyInfo->addTopLevelItem(jobExecutionDateItem);

        jobResultLabelItem = new QTreeWidgetItem();
        jobResultLabelItem->setText(0, tr("Result file:"));
        _ui->_TRW_assemblyInfo->addTopLevelItem(jobResultLabelItem);


        QStringList resultImages = _currentJob->executionDefinition()->resultFileNames();
        nbOfImageFiles = resultImages.size();

        jobResultImageItems = new QTreeWidgetItem *[nbOfImageFiles];

        // display result
        QMessageBox::StandardButton reply;
        reply = QMessageBox::question(this, tr("Load result ?"), tr("Do you want to load result in Matisse (can take time for big reconstructions) ?"),
                                      QMessageBox::Yes|QMessageBox::No);

        for (int i = 0 ; i < nbOfImageFiles ; i++) {

            QString resultFile = resultImages[i];

            QTreeWidgetItem *jobResultItem = new QTreeWidgetItem();
            jobResultItem->setText(0, resultFile);
            jobResultItem->setToolTip(0, resultFile);
            _ui->_TRW_assemblyInfo->addTopLevelItem(jobResultItem);
            jobResultImageItems[i] = jobResultItem;

            if (resultFile.isEmpty()) {
                qCritical() << "Job marked as executed but result filename is empty";
                continue;
            }

            if (reply == QMessageBox::Yes)
                loadResultToCartoView(resultFile, false);
        }
    }

    /* resize first column */
    _ui->_TRW_assemblyInfo->header()->resizeSection(0, graph_chart.dpiScaled(ASSEMBLY_PROPS_LABEL_WIDTH));
    if (jobCommentItem) {
        jobCommentLabelItem->setFirstColumnSpanned(true);
        jobCommentItem->setFirstColumnSpanned(true);
        /* allow multiline comment display */
        _ui->_TRW_assemblyInfo->setItemWidget(jobCommentItem, 0, jobCommentItemText);
    }

    if (jobResultLabelItem) {
        jobResultLabelItem->setFirstColumnSpanned(true);

        for (int i = 0 ; i < nbOfImageFiles ; i++) {
            QTreeWidgetItem *item = jobResultImageItems[i];
            item->setFirstColumnSpanned(true);
        }
    }

    /* memory deallocation for the array of QTreeWidgetItem* */
    delete jobResultImageItems;
}

void AssemblyGui::selectAssembly(QString assemblyName, bool reloadAssembly)
{
    bool found = false;

    for (int indexAssembly = 0; indexAssembly < _ui->_TRW_assemblies->topLevelItemCount(); indexAssembly++) {
        QTreeWidgetItem * assemblyItem = _ui->_TRW_assemblies->topLevelItem(indexAssembly);

        QString currentAssemblyActualName = getActualAssemblyOrJobName(assemblyItem);

        if (currentAssemblyActualName == assemblyName) {
            // selecting item
            found = true;
            _ui->_TRW_assemblies->setCurrentItem(assemblyItem);

            if (reloadAssembly) {
                displayAssembly(assemblyName);
            }

            break;
        }
    }

    if (!found) {
        qCritical() << QString("Assembly '%1' was not found, selection impossible.").arg(assemblyName);
    }
}

void AssemblyGui::selectJob(QString jobName, bool reloadJob)
{
    bool found = false;

    // on parcourt l'arbre à  la recherche du job
    for (int indexAssembly = 0; indexAssembly < _ui->_TRW_assemblies->topLevelItemCount(); indexAssembly++) {
        QTreeWidgetItem * assemblyItem = _ui->_TRW_assemblies->topLevelItem(indexAssembly);
        for (int indexJob = 0; indexJob < assemblyItem->childCount(); indexJob++) {
            QTreeWidgetItem * jobItem = assemblyItem->child(indexJob);
            QString currentJobActualName = getActualAssemblyOrJobName(jobItem);

            if (currentJobActualName == jobName) {
                // selection de l'item...
                found = true;
                _ui->_TRW_assemblies->setCurrentItem(jobItem);
                if (reloadJob) {
                    displayJob(jobName);
                }
                break;
            }
        }
        if (found)
            break;
    }

    if (!found) {
        qCritical() << QString("Job '%1' was not found, selection impossible.").arg(jobName);
    }
}


void AssemblyGui::deleteAssemblyAndReload(bool promptUser)
{
    QTreeWidgetItem * selectedItem = _ui->_TRW_assemblies->currentItem();
    if (!selectedItem) {
        // rien de sélectionné...
        qCritical() << "No item was selected";
        return;
    }

    if (selectedItem->childCount() > 0) {
        // jobs exist... removal prohibited
        // this cas should not occur : controlled at contextual menu creation
        qCritical() << "Jobs are attached to the assembly, it cannot be removed.";
        return;
    }

    QString assemblyName = getActualAssemblyOrJobName(selectedItem);

    if (_processDataManager->assemblyHasArchivedJob(assemblyName)) {
        QMessageBox::critical(this, tr("Invalid path"), tr("Processing chain has archived task and cannot be removed.").arg(assemblyName));
        return;
    }

    if (promptUser) {
        int ret = QMessageBox::question(this, tr("Delete assembly"), tr("Do you want to delete assembly %1 ?").arg(assemblyName), QMessageBox::Ok | QMessageBox::Cancel, QMessageBox::Cancel);
        if (ret==QMessageBox::Cancel) {
            return;
        }
    }

    if (_newAssembly) {
        QString newAssemblyName = getActualNewAssemblyName();

        if (newAssemblyName == assemblyName) { // ensure assembly selected for deletion is the new assembly
            qDebug() << QString("Removing new assembly '%1'").arg(assemblyName);
            _newAssembly->deleteLater();
            _newAssembly = NULL;
            slot_assembliesReload();

            // Reactivate Assembly->Create menu item
            _createAssemblyAct->setEnabled(true);

            return;
        } else {
            // this case should not occur because it is handled at selection
            qCritical() << QString("A new assembly '%1' was not saved, assembly '%2' cannot be deleted").arg(newAssemblyName).arg(assemblyName);
            return;
        }
    }

    AssemblyDefinition *assembly = _processDataManager->getAssembly(assemblyName);
    if (!assembly) {
        qCritical() << QString("Assembly '%1' selected to be removed was not found in local repository").arg(assemblyName);
        return;
    }

    qDebug() << "Removing assembly and assembly parameters files...";

    QString filename = _processDataManager->getAssembly(assemblyName)->filename();
    // suppression fichier assemblage
    QString assemblyFilepath = _processDataManager->getAssembliesPath() + QDir::separator() + filename;
    QString paramFilepath = _processDataManager->getAssembliesParametersPath() + QDir::separator() + filename;

    QFile file(assemblyFilepath);
    if (!file.exists()) {
        qCritical() << QString("Assembly file '%1' does not exist, cannot be removed").arg(assemblyFilepath);
        return;
    }

    if (!file.remove()) {
        qCritical() << QString("Error removing assembly file '%1'").arg(assemblyFilepath);
        return;
    }

    // La suppression du fichier de parametre est controllee mais pas bloquante en cas d'echec
    QFile paramFile(paramFilepath);
    if (paramFile.exists()) {
        if (paramFile.remove()) {
            qDebug() << "... done";
        } else {
            qCritical() << QString("Error removing assembly parameters file '%1'").arg(paramFilepath);
        }
    } else {
        qCritical() << QString("Assembly parameters file '%1' does not exist, cannot be removed").arg(paramFilepath);
    }

    // suppression liste: rechargement...
    slot_assembliesReload();

}



void AssemblyGui::slot_deleteAssembly()
{
    qDebug() << "Delete selected assembly";

    deleteAssemblyAndReload(true);
}

void AssemblyGui::slot_newJob()
{
    // fold parameters widget so the dialog is displayed on top of the dark background
    doFoldUnfoldParameters(false);

    QTreeWidgetItem * item = _ui->_TRW_assemblies->currentItem();
    if (!item) {
        qCritical() << "No assembly selected !";
        return;
    }

    QString assemblyName = item->text(0);

    AssemblyDefinition * assembly = _processDataManager->getAssembly(assemblyName);

    if (!assembly) {
        qCritical() << QString("Assembly '%1' not found in repository").arg(assemblyName);
        return;
    }

    qDebug() << "Creating new job";
    KeyValueList kvl;
    kvl.append("name");
    kvl.append("comment");

    if (!assembly->isRealTime()) {
        kvl.append(DATASET_PARAM_DATASET_DIR);
        kvl.append(DATASET_PARAM_NAVIGATION_FILE);
    }

    /* Key value list is initialized with default dataset preferences */
    kvl.set(DATASET_PARAM_OUTPUT_DIR, _preferences->defaultResultPath());
    kvl.set(DATASET_PARAM_OUTPUT_FILENAME, _preferences->defaultMosaicFilenamePrefix());

    /* Dataset parameters are loaded / overriden from template assembly parameters */
    _server.parametersManager()->pullDatasetParameters(kvl);

    QString jobsBasePath = _processDataManager->getJobsBasePath();
    QStringList jobNames = _processDataManager->getJobsNames();
    QStringList archivedJobNames = _processDataManager->getArchivedJobNames();

    JobDialog dialog(this, _iconFactory, &kvl, jobsBasePath, jobNames, archivedJobNames);
    dialog.setFixedHeight(GraphicalCharter::instance().dpiScaled(JD_HEIGHT));
    dialog.setFixedWidth(GraphicalCharter::instance().dpiScaled(JD_WIDTH));
    dialog.setWindowFlags(Qt::Dialog | Qt::FramelessWindowHint);
    if (dialog.exec() != QDialog::Accepted) {
        return;
    }

    QString assemblyVersion = assembly->version().simplified();

    QString jobName = kvl.getValue("name");

    JobDefinition newJob(jobName, assemblyName, assemblyVersion);
    newJob.setComment(kvl.getValue("comment"));

    ExecutionDefinition executionDefinition;
    executionDefinition.setExecuted(false);
    newJob.setExecutionDefinition(&executionDefinition);

    if (!_processDataManager->writeJobFile(&newJob)) {
        qCritical() << QString("Job definition file could not be created for new job '%1'").arg(jobName);
        return;
    }

    _server.parametersManager()->createJobParametersFile(assemblyName, jobName, kvl);

    // load job definition to local repository
    QString jobFilename = jobName;
    jobFilename.append(".xml");
    _processDataManager->readJobFile(jobFilename);
    JobDefinition* newJobDef = _processDataManager->getJob(jobName);

    // display and select new job (flag is set to false because job was already saved)
    if (addJobInTree(newJobDef, false)) {
        selectJob(jobName);
    }

    _server.parametersManager()->toggleReadOnlyMode(false); // enable parameters editing
    doFoldUnfoldParameters(true);

    _context.setLastActionPerformed(SAVE_JOB);
    enableActions();
}

void AssemblyGui::slot_saveJob()
{
    qDebug() << "Save current job";

    QTreeWidgetItem * item = _ui->_TRW_assemblies->currentItem();
    if (!item) {
        qCritical() << "No job selected";
        return;
    }


    if (!_currentJob) {
        qWarning() << "Current job not identified";

        QString jobName = getActualAssemblyOrJobName(item);

        _currentJob = _processDataManager->getJob(jobName);

        if (!_currentJob) {
            qCritical() << QString("Selected job '%1' not found in local repository").arg(jobName);
            return;
        }
    }

    // modif logique: on peut enregistrer un job deja execute: on enleve l'etat executed et on ecrase...
    if (_currentJob->executionDefinition()->executed()) {
        if (QMessageBox::No == QMessageBox::question(this, tr("Processed job..."),
                                                     tr("Job already processed.\n Do you still want to reprocess ?"),
                                                     QMessageBox::Yes,
                                                     QMessageBox::No)) {
            return;
        }
        _currentJob->executionDefinition()->setExecuted(false);
        _currentJob->executionDefinition()->setExecutionDate(QDateTime());
        _currentJob->executionDefinition()->setResultFileNames(QStringList() << "");
        // changement etat led + effacement image

        if (item->type() == MatisseTreeItem::Type) {
            MatisseTreeItem *jobItem = static_cast<MatisseTreeItem *>(item);
            IconizedWidgetWrapper *itemWrapper = new IconizedTreeItemWrapper(jobItem, 0);
            _iconFactory->attachIcon(itemWrapper, "lnf/icons/led.svg", false, false, _colorsByLevel.value(IDLE));
        } else {
            qCritical() << QString("Item for job '%1' is not of type MatisseTreeItem, cannot display icon").arg(_currentJob->name());
        }

        //        item->setIcon(0, *greyLedIcon);
        _userFormWidget->clear();
    }


    /* save job and parameters */
    if (!_processDataManager->writeJobFile(_currentJob, true)) {
        showError(tr("Job file..."), tr("File %1 could not be write...").arg(_currentJob->name() + ".xml"));
        return;
    }

    _server.parametersManager()->saveParametersValues(_currentJob->name(), false);

    // reinit flag and job displayed name
    _jobParameterModified = false;
    handleJobModified();

    // re-select job
    displayJob(_currentJob->name(), true);
}


void AssemblyGui::slot_deleteJob()
{
    qDebug() << "Delete current job";
    QTreeWidgetItem * selectedItem = _ui->_TRW_assemblies->currentItem();
    if (!selectedItem) {
        // rien de sélectionné...
        return;
    }
    if (selectedItem->parent() == NULL) {
        // assemblage... impossible de supprimer
        // traité ailleurs normalement...
        return;
    }

    QString jobName = getActualAssemblyOrJobName(selectedItem);

    int ret = QMessageBox::question(this, tr("Delete Job"), tr("Do you want to delete Job %1?").arg(jobName), QMessageBox::Ok | QMessageBox::Cancel, QMessageBox::Cancel);
    if (ret==QMessageBox::Cancel) {
        return;
    }
    QString filename = _processDataManager->getJob(jobName)->filename();
    // suppression fichier job

    filename = _processDataManager->getJobsBasePath() + QDir::separator() + filename;
    QFile file(filename);
    if (!file.exists()) {
        qCritical() << QString("Job file '%1' not found, impossible to remove").arg(filename);
        return;
    }

    if (!file.remove()) {
        qCritical() << QString("Job file '%1' could not be removed").arg(filename);
        return;
    }

    filename = _processDataManager->getJobParametersFilePath(jobName);

    QFile parameterFile(filename);

    if (!parameterFile.exists()) {
        qCritical() << QString("Job parameters file '%1' not found, impossible to remove").arg(filename);
    } else {
        if (!parameterFile.remove()) {
            qCritical() << QString("Job parameters file '%1' could not be removed").arg(filename);
        }
    }

    _userFormWidget->resetJobForm();
    // message effacement OK
    // suppression liste: rechargement...
    slot_assembliesReload();
    doFoldUnfoldParameters(false);
}

void AssemblyGui::slot_assemblyContextMenuRequested(const QPoint &pos)
{
    QTreeWidgetItem* item = _ui->_TRW_assemblies->itemAt(pos);

    if (!item) {
        qWarning() << "Right click on assembly tree widget : no item selected";
        return;
    }

    QMenu* contextMenu = new QMenu(_ui->_TRW_assemblies);

    if (!item->parent()) { // menu contextuel pour traitement
        contextMenu->addAction(_createJobAct);
        contextMenu->addAction(_importJobAct);
        contextMenu->addAction(_cloneAssemblyAct);
        // Assemblies cannot be deleted if they have jobs attached
        if (item->childCount() == 0) {
            contextMenu->addAction(_deleteAssemblyAct);
        }

        QString assemblyName = item->text(0);
        if (_processDataManager->assemblyHasArchivedJob(assemblyName)) {
            contextMenu->addAction(_restoreJobAct);
        }

        contextMenu->addAction(_updateAssemblyPropertiesAct);

    } else {              // menu contextuel pour tâche
        if (!_currentJob) {
            qCritical() << "Activated context menu on job but current job is unavailable";
            return;
        }

        contextMenu->addAction(_executeJobAct);
        if (_currentJob->executionDefinition()->executed()) {
            // show only if job was executed
            contextMenu->addAction(_goToResultsAct);
        }
        contextMenu->addSeparator();
        contextMenu->addAction(_uploadDataAct);
        contextMenu->addAction(_executeRemoteJobAct);
        contextMenu->addSeparator();
        contextMenu->addAction(_saveJobAct);
        contextMenu->addAction(_cloneJobAct);
        contextMenu->addAction(_exportJobAct);
        contextMenu->addAction(_deleteJobAct);
        contextMenu->addAction(_archiveJobAct);
    }

    contextMenu->popup(_ui->_TRW_assemblies->viewport()->mapToGlobal(pos));

}

void AssemblyGui::showError(QString title, QString message) {
    qCritical() << title << " - " << message;
    QMessageBox::warning(this, title, message);
}

QTreeWidgetItem *AssemblyGui::addAssemblyInTree(AssemblyDefinition * assembly)
{
    QTreeWidgetItem * assemblyItem = new QTreeWidgetItem(QStringList() << assembly->name());
    assemblyItem->setData(0, Qt::UserRole,assembly->filename());
    _ui->_TRW_assemblies->addTopLevelItem(assemblyItem);
    _assembliesItems.insert(assembly->name(), assemblyItem);

    KeyValueList *props = new KeyValueList();
    props->insert("RealTime", QString::number(assembly->isRealTime()));
    props->insert("Version", assembly->version());
    props->insert("Valid", QString::number(assembly->usable()));
    props->insert("Author", assembly->author());
    props->insert("Comments", assembly->comment());

    /* if new assembly (suffixed by '*'), properties are indexed by the actual assembly name */
    QString actualAssemblyName = getActualAssemblyOrJobName(assemblyItem);
    _assembliesProperties.insert(actualAssemblyName, props);

    return assemblyItem;
}

QTreeWidgetItem *AssemblyGui::addJobInTree(JobDefinition * job, bool isNewJob)
{
    bool executed = false;

    QString assembly = job->assemblyName();

    if (job->executionDefinition() == NULL) {
        qWarning() << "Job definition not found";
        executed = false;
    } else {
        executed = job->executionDefinition()->executed();
    }

    QTreeWidgetItem * assemblyItem = _assembliesItems.value(assembly, NULL);
    if (!assemblyItem) {
        // L'assemblage n'existe pas...
        return NULL;
    }

    QString jobName = job->name();

    if (isNewJob) {
        jobName.append("*");
    }

    MatisseTreeItem *jobItem = new MatisseTreeItem(assemblyItem, QStringList() << jobName);
    jobItem->setData(0, Qt::UserRole, assembly);
    IconizedWidgetWrapper *itemWrapper = new IconizedTreeItemWrapper(jobItem, 0);

    if (executed) {
        _iconFactory->attachIcon(itemWrapper, "lnf/icons/led.svg", false, false, _colorsByLevel.value(OK));

    } else {
        _iconFactory->attachIcon(itemWrapper, "lnf/icons/led.svg", false, false, _colorsByLevel.value(IDLE));
    }

    return jobItem;
}

void AssemblyGui::initStatusBar()
{
    _statusMessageWidget = new StatusMessageWidget(this, _iconFactory);
    _statusMessageWidget->setObjectName("_WID_statusMessage");
    statusBar()->addPermanentWidget(_statusMessageWidget, 10);
}

void AssemblyGui::showStatusMessage(QString message, MessageIndicatorLevel level)
{
    if (message.isEmpty()) {
        return;
    }

    QString levelColorAlias = _colorsByLevel.value(level);
    _statusMessageWidget->addMessage(message, "lnf/icons/led.svg", levelColorAlias);

    emit signal_updateExecutionStatusColor(_colorsByLevel.value(level));
}


void AssemblyGui::initLanguages()
{
    _serverTranslator_en = new QTranslator();
    _serverTranslator_en->load("i18n/MatisseServer_en");

    _serverTranslator_fr = new QTranslator();
    _serverTranslator_fr->load("i18n/MatisseServer_fr");

    _toolsTranslator_en = new QTranslator();
    _toolsTranslator_en->load("i18n/MatisseTools_en");

    _toolsTranslator_fr = new QTranslator();
    _toolsTranslator_fr->load("i18n/MatisseTools_fr");

    // Langue par défaut : Français
    _currentLanguage = "FR";
    qApp->installTranslator(_serverTranslator_fr);
    qApp->installTranslator(_toolsTranslator_fr);
}

void AssemblyGui::updateLanguage(QString language, bool forceRetranslation)
{
    if (language == _currentLanguage) {
        if (forceRetranslation) {
            retranslate();
        } else {
            qDebug("No language change");
        }
        return;
    }

    _currentLanguage = language;

    if (language == "EN") {
        qDebug() << "Translating UI to English";

        qApp->removeTranslator(_serverTranslator_fr);
        qApp->removeTranslator(_toolsTranslator_fr);
        qApp->installTranslator(_serverTranslator_en);
        qApp->installTranslator(_toolsTranslator_en);
    } else {
        qDebug() << "Restoring UI to French";

        qApp->removeTranslator(_serverTranslator_en);
        qApp->removeTranslator(_toolsTranslator_en);
        qApp->installTranslator(_serverTranslator_fr);
        qApp->installTranslator(_toolsTranslator_fr);
    }
}

void AssemblyGui::retranslate()
{
    qDebug() << "Translating static and contextual menu items...";

    /* MENU FICHIER */
    _fileMenu->setTitle(tr("FILE"));
    _exportMapViewAct->setText(tr("Export view to image"));
    _exportProjectQGisAct->setText(tr("Export project to QGIS file"));
    _closeAct->setText(tr("Close"));

    /* MENU AFFICHAGE */
    _displayMenu->setTitle(tr("DISPLAY"));
    _dayNightModeAct->setText(tr("Day/night mode"));
    _mapToolbarAct->setText(tr("Toolbar"));

    /* MENU TRAITEMENTS */
    _processMenu->setTitle(tr("PROCESSING"));
    _createAssemblyAct->setText(tr("Create"));
    _saveAssemblyAct->setText(tr("Save"));
    _importAssemblyAct->setText(tr("Import"));
    _exportAssemblyAct->setText(tr("Export"));

    /* MENU OUTILS */
    _toolMenu->setTitle(tr("TOOLS"));
    _appConfigAct->setText(tr("Configure settings for application"));
    _preprocessingTool->setText(tr("Launch preprocessing tool"));
    //_videoToImageToolAct->setText(tr("Lancer outil transformation de videos en jeux d'image"));
    _checkNetworkRxAct->setText(tr("Check network reception"));

    /* Sous-menu Cartographie */
    _mapMenu->setTitle(tr("Cartography"));
    _loadShapefileAct->setText(tr("Load shapefile"));
    _loadRasterAct->setText(tr("Load raster"));

    /* MENU AIDE */
    _helpMenu->setTitle(tr("HELP"));
    _userManualAct->setText(tr("User manual"));
    _aboutAct->setText(tr("About"));


    /* Menu contextuel Traitement */
    _createJobAct->setText(tr("Create new task"));
    _importJobAct->setText(tr("Import task"));
    _cloneAssemblyAct->setText(tr("Copy"));
    _deleteAssemblyAct->setText(tr("Delete processing chain"));
    _restoreJobAct->setText(tr("Restore"));
    _updateAssemblyPropertiesAct->setText(tr("Update properties"));

    /* Menu contextuel Tâche */
    _executeJobAct->setText(tr("Run"));
    _executeRemoteJobAct->setText(tr("Run on DATARMOR"));
    _uploadDataAct->setText(tr("Upload data to DATARMOR"));
    _saveJobAct->setText(tr("Save"));
    _cloneJobAct->setText(tr("Copy"));
    _exportJobAct->setText(tr("Export"));
    _deleteJobAct->setText(tr("Delete"));
    _archiveJobAct->setText(tr("Archive"));
    _goToResultsAct->setText(tr("Open reconstruction folder"));
}


// Dynamic translation
void AssemblyGui::changeEvent(QEvent *event)
{
    if (event->type() == QEvent::LanguageChange)
    {
        // retranslate designer form
        qDebug() << "Retranslating UI...";
        _ui->retranslateUi(this);
        retranslate();
    }
}

void AssemblyGui::slot_maximizeOrRestore()
{
    if (isMaximized()) {
        showNormal();
        _iconFactory->attachIcon(_maxOrRestoreButtonWrapper, "lnf/icons/agrandir.svg", false, false);
        _maximizeOrRestoreButton->setToolTip(tr("Maximize main window"));
    } else {
        showMaximized();
        _iconFactory->attachIcon(_maxOrRestoreButtonWrapper, "lnf/icons/reinittaille.svg", false, false);
        _maximizeOrRestoreButton->setToolTip(tr("Restore window size"));
    }
}

void AssemblyGui::slot_quit()
{
    bool confirmAction = true;

    if (!_isMapView) {
        if (_isAssemblyModified || _newAssembly) {
            confirmAction = promptAssemblyNotSaved();
        }
    }

    if (_isMapView && _jobParameterModified) {
        promptJobNotSaved();
    }

    if (confirmAction) {
        close();
    }
}

void AssemblyGui::slot_moveWindow(const QPoint &pos)
{
    move(pos);
}

void AssemblyGui::slot_clearAssembly()
{
    //_expertFormWidget -> resetAssemblyForm();
    _ui->_LW_inputs->clearSelection();
    _ui->_LW_processors->clearSelection();
    _ui->_LW_outputs->clearSelection();

    _expertFormWidget->resetAssemblyForm();
    _ui->_TW_creationViewTabs->setCurrentIndex(0);
}

void AssemblyGui::slot_newAssembly()
{
    // fold parameters widget so the dialog is displayed on top of the dark background
    doFoldUnfoldParameters(false);

    slot_clearAssembly();
    _server.parametersManager()->restoreParametersDefaultValues();

    QString name;
    KeyValueList fields;

    AssemblyDialog dialog(this, name, fields, true);
    dialog.setWindowFlags(Qt::Dialog | Qt::FramelessWindowHint);
    if (dialog.exec() != QDialog::Accepted) {
        return;
    }

    qDebug() << "Creating assembly : " << name;
    foreach (QString key, fields.getKeys()) {
        qDebug() << "Key: " << key << " - Value: " << fields.getValue(key);
    }

    // Ajout d'un caractère '*' à la fin du nom pour indiquer qu'il n'est pas enregistré
    QString displayName = name;
    displayName.append('*');

    _newAssembly = new AssemblyDefinition();
    _newAssembly->setName(displayName);
    _newAssembly->setCreationDate(QDate::fromString(fields.getValue("Date")));
    _newAssembly->setDate(fields.getValue("Date"));
    _newAssembly->setIsRealTime(QVariant(QString(fields.getValue("RealTime"))).toBool());
    _newAssembly->setVersion(fields.getValue("Version"));
    _newAssembly->setAuthor(fields.getValue("Author"));
    _newAssembly->setUsable(fields.getValue("Valid").toInt());
    _newAssembly->setComment(fields.getValue("Comments"));

    // filename is derived from the assembly name
    QString filename = _processDataManager->fromNameToFileName(name);
    _newAssembly->setFilename(filename);

    QTreeWidgetItem *item = addAssemblyInTree(_newAssembly);

    _ui->_TRW_assemblies->setCurrentItem(item);
    item->setSelected(true);

    _currentAssembly = NULL;

    //slot_selectAssemblyOrJob(item);
    displayAssemblyProperties(_newAssembly);

    // unfold parameters
    doFoldUnfoldParameters(true);

    _context.setLastActionPerformed(CREATE_ASSEMBLY);
    enableActions();

    // activer vue graphique
    _expertFormWidget->getGraphicsView()->setEnabled(true);

    //    // activer / désactiver menus
    //    _createAssemblyAct->setEnabled(false);
    //    _saveAssemblyAct->setEnabled(true);

    //    // activer / désactiver menus contextuels
    //    _cloneAssemblyAct->setVisible(false);
    //    _updateAssemblyPropertiesAct->setVisible(false);
}

void AssemblyGui::enableActions()
{
    UserAction lastAction = _context.lastActionPerformed();

    /* enable/diable main menu items */
    _exportMapViewAct->setEnabled(_isMapView);
    _exportProjectQGisAct->setEnabled(_isMapView);
    _mapToolbarAct->setEnabled(_isMapView);
    _preprocessingTool->setEnabled(_isMapView);
    //_videoToImageToolAct->setEnabled(_isMapView);
    _loadShapefileAct->setEnabled(_isMapView);
    _loadRasterAct->setEnabled(_isMapView);
    //_createAssemblyAct->setEnabled(!_isMapView);
    //_saveAssemblyAct->setEnabled(!_isMapView);
    _checkNetworkRxAct->setEnabled(_isMapView);

    if (lastAction == SELECT_ASSEMBLY || lastAction == SAVE_ASSEMBLY) {
        _exportAssemblyAct->setEnabled(true);
    } else {
        _exportAssemblyAct->setEnabled(false);
    }

    if (lastAction == CREATE_ASSEMBLY) {
        _createAssemblyAct->setEnabled(false);
        _saveAssemblyAct->setEnabled(false);
        _importAssemblyAct->setEnabled(false);

        _cloneAssemblyAct->setVisible(false);
        _updateAssemblyPropertiesAct->setVisible(false);
    } else if (lastAction == MODIFY_ASSEMBLY){
        _createAssemblyAct->setEnabled(false);
        _saveAssemblyAct->setEnabled(_isAssemblyComplete);
        _importAssemblyAct->setEnabled(false);

        _cloneAssemblyAct->setVisible(false);
        _updateAssemblyPropertiesAct->setVisible(false);
    } else {
        _createAssemblyAct->setEnabled(!_isMapView);
        _saveAssemblyAct->setEnabled(false);
        _importAssemblyAct->setEnabled(true);

        _cloneAssemblyAct->setVisible(!_isMapView);
        _updateAssemblyPropertiesAct->setVisible(!_isMapView);
    }

    /* show / hide assembly contextual menu items */
    _createJobAct->setVisible(_isMapView);
    _importJobAct->setVisible(_isMapView);
    //_cloneAssemblyAct->setVisible(!_isMapView);
    //_updateAssemblyPropertiesAct->setVisible(!_isMapView);
    _restoreJobAct->setVisible(_isMapView);
    _deleteAssemblyAct->setVisible(!_isMapView);

    /* Job contextual menu items are always active as jobs are only visible in the map view */
}

SourceWidget *AssemblyGui::getSourceWidget(QString name)
{
    SourceWidget * wid = _availableSources.value(name, 0);

    if (!wid) {
        return 0;
    }

    _server.addParametersForImageProvider(name);

    SourceWidget * newWidget = new SourceWidget();
    newWidget->clone(wid);

    return newWidget;
}

ProcessorWidget *AssemblyGui::getProcessorWidget(QString name)
{
    ProcessorWidget * wid = _availableProcessors.value(name, 0);

    if (!wid) {
        return 0;
    }

    _server.addParametersForProcessor(name);

    ProcessorWidget * newWidget = new ProcessorWidget();
    newWidget->clone(wid);

    return newWidget;
}

DestinationWidget *AssemblyGui::getDestinationWidget(QString name)
{
    DestinationWidget * wid = _availableDestinations.value(name, 0);

    if (!wid) {
        return 0;
    }

    _server.addParametersForRasterProvider(name);

    DestinationWidget * newWidget = new DestinationWidget();
    newWidget->clone(wid);

    return newWidget;
}


void AssemblyGui::applyNewApplicationContext()
{
    bool isExpert = (_activeApplicationMode == PROGRAMMING);
    _server.parametersManager()->applyApplicationContext(isExpert, !_isMapView);
}

void AssemblyGui::slot_swapMapOrCreationView()
{
    qDebug() << "Current view displayed : " << ((_isMapView) ? "map" : "creation");

    if (_isMapView) {
        if (_jobParameterModified) {
            promptJobNotSaved();
        }

        qDebug() << "Swapping to creation view";

        _isMapView = false;
        if (_activeApplicationMode == PROGRAMMING) {
            _activeViewOrModeLabel->setText(tr("View : Creation"));
        }
        //        _visuModeButton->setIcon(_creationVisuModeIcon);
        _iconFactory->attachIcon(_visuModeButtonWrapper, "lnf/icons/Clef.svg", false, false);
        _visuModeButton->setToolTip(tr("Switch view to cartography"));

        // swap view (1: creation view)
        _ui->_SW_viewStack->setCurrentIndex(1);

        _ui->_TRW_assemblies->collapseAll();
        _ui->_TRW_assemblies->setItemsExpandable(false);
        slot_clearAssembly();
        // on désactive la vue graphique initialement
        _expertFormWidget->getGraphicsView()->setEnabled(false);
        _ui->_TW_creationViewTabs->setCurrentIndex(0);

        // enable parameters editing
        _server.parametersManager()->toggleReadOnlyMode(false);

    } else {
        if (_isAssemblyModified || _newAssembly) {
            bool confirmAction = promptAssemblyNotSaved();

            if (!confirmAction) {
                return;
            }

            _isAssemblyModified = false;
            handleAssemblyModified();
        }

        qDebug() << "Swapping to map view";
        _isMapView = true;
        if (_activeApplicationMode == PROGRAMMING) {
            _activeViewOrModeLabel->setText(tr("View : Cartography"));
        }
        //        _visuModeButton->setIcon(_mapVisuModeIcon);
        _iconFactory->attachIcon(_visuModeButtonWrapper, "lnf/icons/Cartographie.svg", false, false);
        _visuModeButton->setToolTip(tr("Switch view to Creation"));

        /* Reset visible parameters */
        _server.parametersManager()->clearExpectedParameters();

        // swap view (0: map view)
        _ui->_SW_viewStack->setCurrentIndex(0);

        // Changement des info bulles
        _ui->_TRW_assemblies->setItemsExpandable(true);
        _ui->_TRW_assemblies->expandAll();
        _userFormWidget->resetJobForm();
        _ui->_TW_mapViewTabs->setCurrentIndex(0);
    }

    _newAssembly = NULL;
    _currentAssembly = NULL;
    _currentJob = NULL;

    _ui->_TRW_assemblies->clearSelection();

    /* select tab set */
    _ui->_SW_helperTabSets->setCurrentIndex(!_isMapView);

    /* hide live process indicators */
    resetOngoingProcessIndicators();

    // Apply new context to parameters widget
    applyNewApplicationContext();
    doFoldUnfoldParameters(false);

    _context.setLastActionPerformed(SWAP_VIEW);
    enableActions();

}


void AssemblyGui::slot_launchJob()
{
    qDebug() << "Launching job...";


    // On teste assemblage ou job
    // Le bouton est actif si le job est selectionnable
    // cad si l'image n'a pas été produite...
    // ou si on a selectionné un assemblage
    // dans ce cas, on sauvegarde le job avant de le lancer...

    QTreeWidgetItem * currentItem = _ui->_TRW_assemblies->currentItem();

    if (!currentItem) {

        qCritical() << "No job selected, impossible to launch";
        return;
    }

    QTreeWidgetItem * parentItem = currentItem->parent();

    if (!parentItem) {
        qCritical() << "An assembly was selected, cannot run job";
        return;
    }

    QString jobName;

    if (!_currentJob) {
        qWarning() << "Current job not identified";

        QString jobName = getActualAssemblyOrJobName(currentItem);

        _currentJob = _processDataManager->getJob(jobName);

        if (!_currentJob) {
            qCritical() << QString("Selected job '%1' not found in local repository, impossible to launch").arg(jobName);
            return;
        }
    } else {
        jobName = _currentJob->name();
    }

    QString assemblyName = _currentJob->assemblyName();

    // on teste si des valeurs de parametres ont ete modifiees
    //    bool paramValuesModified = false;
    //    if (_parametersWidget->hasModifiedValues()) {
    //        paramValuesModified = true;
    //    }

    if (_currentJob->executionDefinition()->executed()) {
        if (QMessageBox::No == QMessageBox::question(this, tr("Job already processed..."),
                                                     tr("Job already processed.\n Process again ?"),
                                                     QMessageBox::Yes,
                                                     QMessageBox::No)) {
            return;
        }
        _userFormWidget->clear();
    }

    // If a parameter was modified, the user is prompted for saving parameter values
    if (_jobParameterModified) {
        if (QMessageBox::No == QMessageBox::question(this, tr("Parameters changed..."),
                                                     tr("One or more parameters were modified.\nDo you want to save task parameters ?"),
                                                     QMessageBox::Yes,
                                                     QMessageBox::No)) {

            qDebug() << "User aborted job execution";
            return;
        } else {
            qDebug() << "Saving job parameters before launch";
            // Save parameter values
            _server.parametersManager()->saveParametersValues(jobName, false);
            _jobParameterModified = false;
            handleJobModified();
        }
    }

    _server.parametersManager()->saveParametersValues(jobName, false);

    AssemblyDefinition * assemblyDef = _processDataManager->getAssembly(assemblyName);
    if (!assemblyDef) {
        qCritical() << "Assembly error" << assemblyName;
        showStatusMessage(tr("Assembly error"), ERROR);
        return;
    }

    QString jobFilename = _currentJob->filename();

    //_userFormWidget->switchCartoViewTo(QImageView);

    qDebug() << "Running job " << jobName;

    QString msg1=tr("File not found.");
    QString msg2=tr("Job file %1 could not be launched").arg(jobName);

    // on recharge le fichier...
    if (!_processDataManager->readJobFile(jobFilename)) {
        qCritical() << QString("Error reading job file for job '%1' before launch").arg(jobName);
        QMessageBox::information(this, msg1, msg2);
        showStatusMessage(msg1+ " " + msg2, ERROR);
        return;
    }

    _currentJob = _processDataManager->getJob(jobName);

    if (!_currentJob) {
        qCritical() << QString("Job '%1' could not be loaded properly before launch").arg(jobName);
        QMessageBox::information(this, msg1, msg2);
        showStatusMessage(msg1+ " " + msg2, ERROR);
        return;
    }

    /* Copy XML files to result path */
    QString resultPath = _server.parametersManager()->getParameterValue(DATASET_PARAM_OUTPUT_DIR);
    _processDataManager->copyJobFilesToResult(jobName, resultPath);

    _lastJobLaunchedItem = currentItem;

    if (_lastJobLaunchedItem->type() == MatisseTreeItem::Type) {
        MatisseTreeItem *lastJobLaunchedIconItem = static_cast<MatisseTreeItem *>(_lastJobLaunchedItem);
        IconizedWidgetWrapper *itemWrapper = new IconizedTreeItemWrapper(lastJobLaunchedIconItem, 0);
        _iconFactory->attachIcon(itemWrapper, "lnf/icons/led.svg", false, false, _colorsByLevel.value(WARNING));
    } else {
        qCritical() << QString("Item for job '%1' is not of type MatisseTreeItem, cannot display icon").arg(jobName);
    }


    QString msg = tr("Job %1 running...").arg(jobName);
    showStatusMessage(msg, IDLE);

    doFoldUnfoldParameters(false);
    _stopButton->setEnabled(true);
    _ongoingProcessInfolabel->show();
    _ongoingProcessCompletion->show();
    emit signal_processRunning();

    bool runSuccess = _server.processJob(*_currentJob);

    if (!runSuccess) {
        QString msg = tr("Error %1: %2").arg(jobName).arg(_server.messageStr());
        showStatusMessage(msg, ERROR);
        _stopButton->setEnabled(false);
    }else{
        freezeJobUserAction(true);
    }

    //setActionsStates(_lastJobLaunchedItem);
}

void AssemblyGui::slot_launchRemoteJob()
{
    qDebug() << "Launching remote job...";

    executeExportWorkflow(true, true);

    _remoteJobHelper->uploadJobFiles(_currentBundleForRemoteExecution);
}

void AssemblyGui::slot_uploadJobData()
{
    qDebug() << "Uploading job data...";

    QFileDialog dialog(this, tr("Select dataset to upload"));
    dialog.setFileMode(QFileDialog::Directory);
    
    if (!dialog.exec()) {
        qCritical() << "File dialog could no execute";
        return;
    }
    
    QStringList filenames = dialog.selectedFiles();
    if (filenames.isEmpty()) {
        qCritical() << "No dataset folder selected, upload could not be performed";
        return;
    }

    QString datasetDirName = filenames.at(0);

    if (datasetDirName.isEmpty()) {
        qCritical() << "Selected dataset folder name is empty, upload could not be performed";
        return;
    }

    QDir datasetDir(datasetDirName);
    if (!datasetDir.exists()) {
        qCritical() << QString("Dataset folder %1 does not exist, upload could not be performed").arg(datasetDirName);
        return;
    }

    QString datasetDirFinal = datasetDir.canonicalPath();

    _remoteJobHelper->uploadDataset(datasetDirFinal);
}


void AssemblyGui::slot_stopJob()
{
    QString jobName = _lastJobLaunchedItem->data(0, Qt::UserRole).toString();
    qDebug() << "Stopping job " << jobName.toLatin1();

    QString msg1=tr("Stopping running job.");
    QString msg2=tr("Do you want to stop or cancel the job?");
    QMessageBox msgBox;
    msgBox.setText(msg1);
    msgBox.setInformativeText(msg2);
    QPushButton *stopButton = msgBox.addButton(tr("Stop"), QMessageBox::AcceptRole);
    QPushButton *discardButton = msgBox.addButton(QMessageBox::Discard);
    msgBox.addButton(QMessageBox::Cancel);

    msgBox.exec();
    if (msgBox.clickedButton() == stopButton) {
        _server.stopJob(false);

        resetOngoingProcessIndicators();;
        emit signal_processStopped();
        _stopButton->setEnabled(false);
    }
    else if (msgBox.clickedButton() == discardButton) {
        _server.stopJob(true);

        resetOngoingProcessIndicators();
        emit signal_processStopped();
        _stopButton->setEnabled(false);
    }
    else {
        // cancel dialog => do nothing
    }

    freezeJobUserAction(false);

}

void AssemblyGui::slot_jobShowImageOnMainView(QString name, Image *image)
{
    Q_UNUSED(name)
    if (image) {
        _userFormWidget->displayImage(image);
        delete image;
    }
}

void AssemblyGui::slot_userInformation(QString userText)
{
    qDebug() << "Received user information : " << userText;
    _ongoingProcessInfolabel->setText(userText);
}

void AssemblyGui::slot_processCompletion(quint8 percentComplete)
{
    qDebug() << "Received process completion signal : " << percentComplete;

    if(percentComplete == (quint8)-1)
    {
        _ongoingProcessCompletion->setTextVisible(false);
        _ongoingProcessCompletion->setValue(0);
    }
    else
    {
        _ongoingProcessCompletion->setTextVisible(true);

        if (percentComplete > 100) {
            qWarning() << QString("Invalid process completion percentage value : %1").arg(percentComplete);
            return;
        }

        _ongoingProcessCompletion->setValue(percentComplete);
    }
}

void AssemblyGui::slot_showInformationMessage(QString title, QString message)
{
    QMessageBox::information(this, title, message);
}

void AssemblyGui::slot_showErrorMessage(QString title, QString message)
{
    QMessageBox::critical(this, title, message);
}


void AssemblyGui::slot_jobProcessed(QString name, bool isCancelled) {
    qDebug() << "Job done : " << name;
    //_userFormWidget->switchCartoViewTo(QGisMapLayer);

    if (!_server.errorFlag()) {
        JobDefinition *jobDef = _processDataManager->getJob(name);
        QDateTime now = QDateTime::currentDateTime();
        jobDef->executionDefinition()->setExecutionDate(now);
        _processDataManager->writeJobFile(jobDef, true);
        if (_lastJobLaunchedItem) {
            if (isCancelled) {
                if (_lastJobLaunchedItem->type() == MatisseTreeItem::Type) {
                    MatisseTreeItem *lastJobLaunchedIconItem = static_cast<MatisseTreeItem *>(_lastJobLaunchedItem);
                    IconizedWidgetWrapper *itemWrapper = new IconizedTreeItemWrapper(lastJobLaunchedIconItem, 0);
                    _iconFactory->attachIcon(itemWrapper, "lnf/icons/led.svg", false, false, _colorsByLevel.value(IDLE));
                } else {
                    qCritical() << QString("Item for job '%1' is not of type MatisseTreeItem, cannot display icon").arg(name);
                }

                QString msg = tr("Job %1 cancelled...").arg(jobDef->name());
                showStatusMessage(msg, OK);
                // TODO Image cleaning ?
            }
            else  {
                // led update...
                if (_lastJobLaunchedItem->type() == MatisseTreeItem::Type) {
                    MatisseTreeItem *lastJobLaunchedIconItem = static_cast<MatisseTreeItem *>(_lastJobLaunchedItem);
                    IconizedWidgetWrapper *itemWrapper = new IconizedTreeItemWrapper(lastJobLaunchedIconItem, 0);
                    _iconFactory->attachIcon(itemWrapper, "lnf/icons/led.svg", false, false, _colorsByLevel.value(OK));
                } else {
                    qCritical() << QString("Item for job '%1' is not of type MatisseTreeItem, cannot display icon").arg(name);
                }

                QString msg = tr("Job %1 finished...").arg(jobDef->name());
                showStatusMessage(msg, OK);

                selectJob(jobDef->name());
            }
        }

        foreach (QString resultFile, jobDef->executionDefinition()->resultFileNames()) {

            if (jobDef->executionDefinition()->executed() && (!resultFile.isEmpty())) {

                // display result
                QMessageBox::StandardButton reply;
                reply = QMessageBox::question(this, tr("Load result ?"), tr("Do you want to load result in Matisse (can take time for big reconstructions) ?"),
                                              QMessageBox::Yes|QMessageBox::No);
                if (reply == QMessageBox::Yes)
                    loadResultToCartoView(resultFile, false);
            }
        }

        // Désactiver le bouton STOP
        emit signal_processStopped();
        _stopButton->setEnabled(false);
        freezeJobUserAction(false);
    }
}

void AssemblyGui::slot_assembliesReload()
{
    loadAssembliesAndJobsLists(_isMapView);

    if (_isMapView) {
        _userFormWidget->clear();
        qDebug() << "Clear userForm...";
    } else {
        slot_clearAssembly();
    }

    _server.parametersManager()->clearExpectedParameters();
    _currentAssembly = NULL;
    _currentJob = NULL;
}

void AssemblyGui::slot_modifiedParameters(bool changed)
{
    qDebug() << "Receiving parameter value update flag : " << changed;

    bool hasActuallyChanged = changed;

    if (!changed) {
        qDebug() << "Canceled parameter value modification";
        hasActuallyChanged = _parametersWidget->hasModifiedValues();

        if (hasActuallyChanged) {
            qDebug() << "But some parameter values remain modified";
        }
    }

    if (_isMapView) {
        _jobParameterModified = hasActuallyChanged;
        handleJobModified();

    } else {
        _isAssemblyModified = hasActuallyChanged;
        handleAssemblyModified();
        if (hasActuallyChanged) {
            _context.setLastActionPerformed(MODIFY_ASSEMBLY);
            enableActions();
        }
        //_createAssemblyAct->setEnabled(!hasActuallyChanged);
    }
}

void AssemblyGui::slot_modifiedAssembly()
{
    qDebug() << "Received assembly modified notification";

    _isAssemblyModified = true;
    handleAssemblyModified();
    //    _createAssemblyAct->setEnabled(false);

    _context.setLastActionPerformed(MODIFY_ASSEMBLY);
    enableActions();
}

void AssemblyGui::slot_assemblyComplete(bool isComplete)
{
    qDebug() << "Received assembly completeness flag : " << isComplete;

    _isAssemblyComplete = isComplete;
    //    _saveAssemblyAct->setEnabled(isComplete); // assembly can be saved only if assembly is complete
}

void AssemblyGui::handleJobModified()
{
    if (!_currentJob) {
        qCritical() << "A job was modified but current job is not found";
        return;
    }

    QTreeWidgetItem * selectedItem = _ui->_TRW_assemblies->currentItem();
    if (!selectedItem) {
        qCritical("Current job item is null (no selection)");
        return;
    }

    QString jobDisplayName = selectedItem->text(0);

    bool markedAsModified = jobDisplayName.right(1) == "*";

    /* mark job as modified */
    if (_jobParameterModified && !markedAsModified) {
        selectedItem->setText(0, jobDisplayName.append("*"));
    }

    /* restore job name */
    if (!_jobParameterModified && markedAsModified) {
        selectedItem->setText(0, _currentJob->name());
    }
}

void AssemblyGui::handleAssemblyModified()
{
    if (_newAssembly) {
        qDebug() << "Updated new assembly " << _newAssembly->name();
        return;
    }

    if (!_currentAssembly) {
        qCritical() << "Notified for current assembly modification but current assembly is not found";
        return;
    }

    QTreeWidgetItem * selectedItem = _ui->_TRW_assemblies->currentItem();
    if (!selectedItem) {
        qCritical("Current assembly item is null (no selection)");
        return;
    }

    QString assemblyDisplayName = selectedItem->text(0);

    bool markedAsModified = assemblyDisplayName.right(1) == "*";

    /* mark assembly as modified */
    if (_isAssemblyModified && !markedAsModified) {
        selectedItem->setText(0, assemblyDisplayName.append("*"));
    }

    /* restore assembly name */
    if (!_isAssemblyModified && markedAsModified) {
        selectedItem->setText(0, _currentAssembly->name());
    }
}

QString AssemblyGui::getActualAssemblyOrJobName(QTreeWidgetItem* currentItem)
{
    QString assemblyOrJobName = currentItem->text(0);
    /* case job parameters were modified */
    /* case assembly assembly was modified (parameters or processing chain) */
    if (assemblyOrJobName.right(1) == "*") {
        assemblyOrJobName.chop(1); // remove trailing '*
    }

    return assemblyOrJobName;
}

QString AssemblyGui::getActualNewAssemblyName()
{
    if (!_newAssembly) {
        qCritical() << "No new assembly identified";
        return "";
    }

    QString assemblyName = _newAssembly->name();
    /* case assembly was modified (parameters or processing chain) : name is suffixed by '*' */
    if (assemblyName.right(1) == "*") {
        assemblyName.chop(1); // remove traing '*'
    } else {
        qWarning() << QString("New assembly name '%1' does not end with '*'").arg(assemblyName);
    }

    return assemblyName;
}


#include "AssemblyGui.h"
#include "ui_AssemblyGui.h"

using namespace MatisseTools;
using namespace MatisseServer;

const QString AssemblyGui::PREFERENCES_FILEPATH = QString("config/MatissePreferences.xml");

AssemblyGui::AssemblyGui(QString settingsFile, QWidget *parent) :

    QMainWindow(parent),
    _ui(new Ui::AssemblyGui),
    _mapVisuModeIcon(":/qss_icons/icons/Cartographie.svg"),
    _creationVisuModeIcon(":/qss_icons/icons/Clef.svg"),
    _maximizeIcon(":/qss_icons/icons/agrandir.svg"),
    _restoreToNormalIcon(":/qss_icons/icons/reinittaille.svg"),
    _currentJob(NULL),
    _currentAssembly(NULL),
    _newAssembly(NULL),
    _isAssemblyComplete(false),
    _assemblyVersionPropertyItem(NULL),
    _assemblyCreationDatePropertyItem(NULL),
    _assemblyAuthorPropertyItem(NULL),
    _assemblyCommentPropertyHeaderItem(NULL),
    _assemblyCommentPropertyItem(NULL),
    _assemblyCommentPropertyItemText(NULL)
{
    _ui->setupUi(this);
    _canShow = setSettingsFile(settingsFile);
    _server.setMainGui(this);

    qRegisterMetaType< basicproc::Polygon >();

    init();
}

AssemblyGui::~AssemblyGui()
{
    delete _ui;
    qDebug() << "Delete Gui";
}

bool AssemblyGui::setSettingsFile(QString settings)
{
    _settingsFile = settings;
    QString standardFile = "config/MatisseSettings.xml";
    // lecture du fichier xml de settings
    if (_settingsFile == "") {
        _settingsFile = standardFile;
    }
    QFileInfo settingsFile(_settingsFile);
    if (!settingsFile.exists()) {
        QString adviceMsg = tr("Impossible de trouver le fichier:\n%1\nRelancez l'application avec un nom de fichier valide en parametre\nou un fichier %2 valide!").arg(_settingsFile).arg(standardFile);
        QMessageBox::critical(this, tr("Fichier de configuration introuvable"), adviceMsg);
        return false;
    }

    if (!settingsFile.isReadable()) {
        QString adviceMsg = tr("Impossible de lire le fichier:\n%1\nRelancez l'application avec un nom de fichier valide en parametre\nou un fichier %2 valide!").arg(_settingsFile).arg(standardFile);
        QMessageBox::critical(this, tr("Fichier de configuration illisible"), adviceMsg);
        return false;
    }

    Xml xmlProcessing;
    xmlProcessing.readMatisseGuiSettings(_settingsFile);
    if ((_rootXml = xmlProcessing.getBasePath()) == "") {
        QString adviceMsg = tr("La valeur de XmlRootDir ne peut etre determinee.\nRelancez l'application avec un parametre XmlRootDir valide\ndans le fichier de configuration!");
        QMessageBox::critical(this, tr("Fichier de configuration incorrect"), adviceMsg);
        return false;
    }

    _appVersion = xmlProcessing.getVersion();

    return true;
}

bool AssemblyGui::isShowable()
{
    return _canShow;
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

void AssemblyGui::initPreferences()
{
    _preferences = new MatissePreferences();
    Xml xmlProcessing;

    QFile prefsFile(PREFERENCES_FILEPATH);

    if (!prefsFile.exists()) {
        // Creating preferences file
        _preferences->setLastUpdate(QDateTime::currentDateTime());
        _preferences->setImportExportPath("data/exchange");
        _preferences->setArchivePath("data/archive");
        _preferences->setDefaultResultPath("data/results");
        _preferences->setDefaultMosaicFilenamePrefix("MaMosaique");
        _preferences->setLanguage("FR");

        xmlProcessing.writeMatissePreferences(PREFERENCES_FILEPATH, *_preferences);
    } else {
        xmlProcessing.readMatissePreferences(PREFERENCES_FILEPATH, *_preferences);
    }

    updateLanguage(_preferences->language(), true);

    updatePreferredDatasetParameters();
}

void AssemblyGui::initParametersWidget()
{
    _parametersDock = _ui->_SCA_parametersDock;

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

void AssemblyGui::init()


{
    initLanguages();

    // INITIALISATION DES STYLESHEET

    // reset window stylesheet property (utilisée uniquement pour preview dans Qt Designer)
    initStylesheetSelection();

    // Recherche des boutons d'action et système
    _visuModeButton = findChild<QToolButton*>(QString("_TBU_visuModeSwap"));
    QToolButton* closeButton = findChild<QToolButton*>(QString("_TBU_closeButton"));
    _maximizeOrRestoreButton = findChild<QToolButton*>(QString("_TBU_maximizeRestoreButton"));
    QToolButton* minimizeButton = findChild<QToolButton*>(QString("_TBU_minimizeButton"));
    _stopButton = findChild<QToolButton*>(QString("_TBU_stopButton"));   

    // Recherche des libellés, pictogrammes ou indicateurs
    _activeViewOrModeLabel = findChild<QLabel*>(QString("_LA_activeView"));
    _currentDateTimeLabel = findChild<QLabel*>(QString("_LA_currentDateTime"));
    _ongoingProcessInfolabel = findChild<QLabel*>(QString("_LA_ongoingProcessInfoLabel"));
    _matisseVersionlabel = findChild<QLabel*>(QString("_LAB_matisseVersion"));
    _ongoingProcessCompletion = findChild<QProgressBar*>(QString("_PB_ongoingProcessCompletion"));
    _liveProcessWheel = findChild<LiveProcessWheel*>(QString("_WID_liveProcessWheel"));

    // Tabs : object name is set anew explicitely to enable stylesheet ( setObjectName overriden)
    QTabWidget *mapViewTabs = findChild<QTabWidget*>(QString("_TW_mapViewTabs"));
    mapViewTabs->setObjectName("_TW_mapViewTabs");
    QTabWidget *creationViewTabs = findChild<QTabWidget*>(QString("_TW_creationViewTabs"));
    creationViewTabs->setObjectName("_TW_creationViewTabs");

    // hide current process indicators
    resetOngoingProcessIndicators();

    // Initialisation du menu principal
    initMainMenu();

    // Initialisation des menus contextuels
    initContextMenus();


    // Recherche autres widgets
    HomeWidget* homeWidget = findChild<HomeWidget*>(QString("homeWidget"));

    _jobParameterModified = false;
    _isAssemblyModified = false;

    _userFormWidget = _ui->_WID_mapViewSceneContainer;
    _expertFormWidget = _ui->_WID_creationSceneContainer;

    connect(this, SIGNAL(signal_processRunning()), _liveProcessWheel, SLOT(slot_processRunning()));
    connect(this, SIGNAL(signal_processStopped()), _liveProcessWheel, SLOT(slot_processStopped()));
    connect(this, SIGNAL(signal_processFrozen()), _liveProcessWheel, SLOT(slot_processFrozen()));
    connect(this, SIGNAL(signal_updateWheelColors(QString)), _liveProcessWheel, SLOT(slot_updateWheelColors(QString)));

    //connect(_ui->_TRW_assemblies, SIGNAL(doubleClicked(QModelIndex)), this, SLOT(slot_showAssembly(QModelIndex)));
    connect(_ui->_TRW_assemblies, SIGNAL(itemClicked(QTreeWidgetItem*,int)), this, SLOT(slot_selectAssemblyOrJob(QTreeWidgetItem*,int)));
    connect(_ui->_TRW_assemblies, SIGNAL(customContextMenuRequested(QPoint)), this, SLOT(slot_assemblyContextMenuRequested(QPoint)));

    connect(_visuModeButton, SIGNAL(clicked()), this, SLOT(slot_swapMapOrCreationView()));

    // Home action
    connect(homeWidget, SIGNAL(signal_goHome()), this, SLOT(slot_goHome()));

    // System actions
    connect(closeButton, SIGNAL(clicked()), this, SLOT(slot_quit()));
    connect(_maximizeOrRestoreButton, SIGNAL(clicked()), this, SLOT(slot_maximizeOrRestore()));
    connect(minimizeButton, SIGNAL(clicked()), this, SLOT(showMinimized()));
    connect(_ui->_MCB_controllBar, SIGNAL(signal_moveWindow(QPoint)), this, SLOT(slot_moveWindow(QPoint)));

    // Menu actions
    connect(_closeAct, SIGNAL(triggered()), this, SLOT(close()));
    connect(_createAssemblyAct, SIGNAL(triggered()), this, SLOT(slot_newAssembly()));
    connect(_saveAssemblyAct, SIGNAL(triggered()), this, SLOT(slot_saveAssembly()));
    connect(_updateAssemblyPropertiesAct, SIGNAL(triggered()), this, SLOT(slot_saveAssembly()));
    connect(_appConfigAct, SIGNAL(triggered()), this, SLOT(slot_updatePreferences()));
    connect(_userManualAct, SIGNAL(triggered()), this, SLOT(slot_showUserManual()));
    connect(_aboutAct, SIGNAL(triggered()), this, SLOT(slot_showAboutBox()));

    // Menus contextuels
    connect(_createJobAct, SIGNAL(triggered()), this, SLOT(slot_newJob()));
    connect(_saveJobAct, SIGNAL(triggered()), this, SLOT(slot_saveJob()));
    connect(_deleteAssemblyAct, SIGNAL(triggered()), this, SLOT(slot_deleteAssembly()));

    connect(_executeJobAct, SIGNAL(triggered()), this, SLOT(slot_launchJob()));
    connect(_deleteJobAct, SIGNAL(triggered()), this, SLOT(slot_deleteJob()));

    // Tool button actions
    connect(_stopButton, SIGNAL(clicked()), this, SLOT(slot_stopJob()));
    connect(_ui->_PB_parameterFold, SIGNAL(clicked(bool)), this, SLOT(slot_foldUnfoldParameters()));


    _server.setSettingsFile();
    _server.init();

    _beforeSelect = true;
    _lastJobLaunchedItem = NULL;

    connect(&_server, SIGNAL(signal_jobProcessed(QString, bool)), this, SLOT(slot_jobProcessed(QString, bool)));
    connect(&_server, SIGNAL(signal_jobShowImageOnMainView(QString,Image *)), this, SLOT(slot_jobShowImageOnMainView(QString,Image *)));
    connect(&_server, SIGNAL(signal_userInformation(QString)), this, SLOT(slot_userInformation(QString)));
    connect(&_server, SIGNAL(signal_processCompletion(quint8)), this, SLOT(slot_processCompletion(quint8)));

    initVersionDisplay();

    initParametersWidget();

    initPreferences();

    _expertFormWidget->setServer(&_server);    
    _expertFormWidget->getScene()->setElementWidgetProvider(this);
    _expertFormWidget->getScene()->setMessageTarget(this);

    connect(_expertFormWidget->getScene(), SIGNAL(signal_assemblyModified()), this, SLOT(slot_modifiedAssembly()));
    connect(_expertFormWidget->getScene(), SIGNAL(signal_assemblyComplete(bool)), this, SLOT(slot_assemblyComplete(bool)));

    // Loading processors
    initProcessorWidgets();

    _isMapView = false; // initialisé à false (création) pour être basculé à true (carto) lors du 1er swap
    slot_swapMapOrCreationView();
    _userFormWidget->createCanvas();

    _newAssembly = NULL;

    //statusBar
    initStatusBar();

    // start current date/time timer
    initDateTimeDisplay();
}

void AssemblyGui::initStylesheetSelection()
{
    setStyleSheet("");

    _stylesheetByAppMode.clear();
    _stylesheetByAppMode.insert(PROGRAMMING, "lnf/MatisseModeProg.css");
    _stylesheetByAppMode.insert(REAL_TIME, "lnf/MatisseModeRt.css");
    _stylesheetByAppMode.insert(DEFERRED_TIME, "lnf/MatisseModeDt.css");
    _stylesheetByAppMode.insert(APP_CONFIG, "lnf/MatisseModeProg.css");

    _stopButtonIconByAppMode.clear();
    _stopButtonIconByAppMode.insert(PROGRAMMING, QIcon(":/qss_icons/icons/Main_mode-prog.svg"));
    _stopButtonIconByAppMode.insert(REAL_TIME, QIcon(":/qss_icons/icons/Main_mode-rt.svg"));
    _stopButtonIconByAppMode.insert(DEFERRED_TIME, QIcon(":/qss_icons/icons/Main_mode-dt.svg"));
    _stopButtonIconByAppMode.insert(APP_CONFIG, QIcon(":/qss_icons/icons/Main_mode-prog.svg"));

    _messagePictoByAppMode.clear();
    _messagePictoByAppMode.insert(PROGRAMMING, QPixmap(":/qss_icons/icons/Message_mode-prog.svg"));
    _messagePictoByAppMode.insert(REAL_TIME, QPixmap(":/qss_icons/icons/Message_mode-rt.svg"));
    _messagePictoByAppMode.insert(DEFERRED_TIME, QPixmap(":/qss_icons/icons/Message_mode-dt.svg"));
    _messagePictoByAppMode.insert(APP_CONFIG, QPixmap(":/qss_icons/icons/Message_mode-prog.svg"));

    _wheelColorsByMode.clear();
    _wheelColorsByMode.insert(PROGRAMMING, QString("%1-%2-%3").arg(MATISSE_GREY).arg(MATISSE_YELLOW_DARK).arg(MATISSE_YELLOW));
    _wheelColorsByMode.insert(REAL_TIME, QString("%1-%2-%3").arg(MATISSE_GREY).arg(MATISSE_BLUE_DARK).arg(MATISSE_BLUE));
    _wheelColorsByMode.insert(DEFERRED_TIME, QString("%1-%2-%3").arg(MATISSE_GREY).arg(MATISSE_MAUVE_DARK).arg(MATISSE_MAUVE));
    _wheelColorsByMode.insert(APP_CONFIG, QString("%1-%2-%3").arg(MATISSE_GREY).arg(MATISSE_YELLOW_DARK).arg(MATISSE_YELLOW));

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
    _mapToolbarAct = new QAction(this);

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
    _exposureToolAct = new QAction(this);
    _videoToImageToolAct = new QAction(this);
    _checkNetworkRxAct = new QAction(this);

    /* Sous-menu Cartographie */
    _loadShapefileAct = new QAction(this);
    _loadRasterAct = new QAction(this);

    _toolMenu->addAction(_appConfigAct);
    _toolMenu->addSeparator();
    _toolMenu->addAction(_exposureToolAct);
    _toolMenu->addAction(_videoToImageToolAct);
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
    mainMenuBar->setMinimumHeight(_helpMenu->height());
    mainMenuBar->setMaximumHeight(_helpMenu->height());
    mainMenuBar->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Fixed);

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
    QString templateLabel = "MATISSE %1\nV%1.%2.%3";

    QStringList versionItems = _appVersion.split(".");

    if (versionItems.size() < 3) {
        qCritical() << QString("Version '%1' defined in XML settings does not conform to the 'X.Y.Z' pattern, default version will be displayed").arg(_appVersion);
        return;
    }

    QString productSerie = versionItems.at(0);
    QString major = versionItems.at(1);
    QString minor = versionItems.at(2);

    QString fullVersionLabel = templateLabel.arg(productSerie).arg(major).arg(minor);

    _matisseVersionlabel->setText(fullVersionLabel);
}

bool AssemblyGui::loadResultToCartoView(QString resultFile_p)
{
    QFileInfo infoResult(resultFile_p);

    if (!infoResult.exists()) {
        qCritical() << "Erreur fichier image introuvable" << infoResult.absoluteFilePath();
    }

    if (_userFormWidget->supportedRasterFormat().contains(infoResult.suffix())){
        _userFormWidget->loadRasterFile(infoResult.absoluteFilePath());

    }else if (_userFormWidget->supportedVectorFormat().contains(infoResult.suffix())){
        _userFormWidget->loadShapefile(infoResult.absoluteFilePath());

    }else if (_userFormWidget->supported3DFileFormat().contains(infoResult.suffix())){
        _userFormWidget->load3DFile(infoResult.absoluteFilePath());

    }else if (_userFormWidget->supportedImageFormat().contains(infoResult.suffix())){
        _userFormWidget->loadImageFile(infoResult.absoluteFilePath());

    }else{
        qDebug() << "Output file format not supported";
        return false;
    }

    return true;

}


// TODO Chargement des jobs à déplacer dans Server ou Xml
void AssemblyGui::loadAssembliesAndJobsLists(bool doExpand)
{
    // parse du repertoire et recuperation des differents jobs
    //_assembliesValues.clear();
    QString jobsPath =  _server.xmlTool().getJobsPath();
    QString assembliesPath = _server.xmlTool().getAssembliesPath();

    qDebug() << "Load assemblies from " + assembliesPath;
    QDir assembliesDir(assembliesPath);
    QStringList assembliesFiles = assembliesDir.entryList(QStringList() << "*.xml");
    _server.xmlTool().clearAssembliesDatas();

    // La liste des topItems
    _ui->_TRW_assemblies->clear();
    _assembliesItems.clear();

    // free properties key value lists memory allocation
    QList<KeyValueList*> props = _assembliesProperties.values();
    foreach (KeyValueList* prop, props) {
        delete prop;
    }
    _assembliesProperties.clear();

    // chargement des assemblies
    foreach(QString assembly, assembliesFiles) {
        qDebug() << "Lecture assembly" << assembly;
        if (!_server.xmlTool().readAssemblyFile(assembly)) {
            qDebug() << "Unable to read assembly " << assembly;
            continue;
        }
    }

    // deuxième passe, création des items...
    // on met le nom du fichier dans le userRole afin
    // de le rendre accessible ailleurs...
    QStringList validAssemblies = _server.xmlTool().getAssembliesList();

    foreach(QString assemblyName, validAssemblies) {
        AssemblyDefinition *assembly = _server.xmlTool().getAssembly(assemblyName);

        // Selon mode, on n'ajoute que les traitements TR ou TD
        if (_activeApplicationMode == REAL_TIME && !assembly->isRealTime()) {
            continue;
        } else if (_activeApplicationMode == DEFERRED_TIME && assembly->isRealTime()) {
            continue;
        }

        addAssemblyInTree(assembly);
    }


    qDebug() << "Load jobs from " + jobsPath;
    QDir jobsDir(jobsPath);
    QStringList jobs  = jobsDir.entryList(QStringList() << "*.xml");
    _server.xmlTool().clearJobsDatas();
    foreach(QString job, jobs) {
        //qDebug() << "Lecture" << job;
        if (!_server.xmlTool().readJobFile(job)) {
            qDebug() << "Unable to read job " << job;
            continue;
        }
    }

    // construction arbre
    QStringList jobsNames = _server.xmlTool().getJobsNames();
    foreach(QString jobName, jobsNames) {
        JobDefinition * jobDef = _server.xmlTool().getJob(jobName);

        if (jobDef) {
            addJobInTree(jobDef);
        }
    }

    _ui->_TRW_assemblies->sortItems(0, Qt::AscendingOrder);
    if (doExpand) {
        _ui->_TRW_assemblies->expandAll();
    }

    // Si un nouvel assemblage était en cours, on le rajoute à l'arbre
    if (_newAssembly) {
        addAssemblyInTree(_newAssembly);
    }
}

void AssemblyGui::loadStyleSheet(ApplicationMode mode)
{
    // sélection de la feuille de style spécifique au mode
    QString styleSheetForMode = _stylesheetByAppMode.value(mode);
    qDebug() << QString("Stylesheet for mode : %1").arg(styleSheetForMode);

    // Chargement des feuilles de style...

    QFile genericStyleSheet("lnf/Matisse.css");
    QFile modeSpecificStyleSheet(styleSheetForMode);

    if (!genericStyleSheet.exists() || !modeSpecificStyleSheet.exists()) {
        qWarning() << "ERROR : stylesheet files not found";
    } else {
        if (!genericStyleSheet.open(QIODevice::ReadOnly)) {
            qWarning() << "ERROR : generic stylesheet file could not be open\n" << genericStyleSheet.error();
        } else {
            QByteArray globalStyles = genericStyleSheet.readAll();
            genericStyleSheet.close();

            if(!modeSpecificStyleSheet.open(QIODevice::ReadOnly)) {
                qWarning() << "ERROR : mode specific stylesheet file could not be open\n" << modeSpecificStyleSheet.error();
            } else {
                // ajout des styles spécifiques à la feuille de style globale
                globalStyles.append(modeSpecificStyleSheet.readAll());
                modeSpecificStyleSheet.close();
            }

            // application de la feuille de style
            qDebug() << "Applying stylesheet...";
            qApp->setStyleSheet(globalStyles);
        }
    }

    // CUSTOMISATIONS HORS FEUILLE DE STYLE

    /* Icônes de boutons ou labels avec couleur de premier plan */
    QToolButton* stopButton = findChild<QToolButton*>(QString("_TBU_stopButton"));
    QIcon stopButtonIcon = _stopButtonIconByAppMode.value(mode);
    stopButton->setIcon(stopButtonIcon);    

    QPixmap messagesPictoIcon = _messagePictoByAppMode.value(mode);
    _messagesPicto->setPixmap(messagesPictoIcon);

    /* Live process wheel */
    QString wheelColors = _wheelColorsByMode.value(mode);
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
        _activeViewOrModeLabel->setText(tr("Mode : Temps reel"));
    } else if(mode == DEFERRED_TIME){
        _activeViewOrModeLabel->setText(tr("Mode : Depouillement"));
    }

    // always init application mode with map view
    if (!_isMapView) {
        slot_swapMapOrCreationView();
    } else {
        if (_activeApplicationMode == PROGRAMMING) {
            _activeViewOrModeLabel->setText(tr("Vue : Cartographie"));
        }
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

        int userResponse = QMessageBox::question(this, tr("Nouvel assemblage..."),
                                        tr("L'assemblage '%1' n'a pas encore ete enregistre.\nContinuer sans enregistrer ?\n(Supprimera le nouvel assemblage)")
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

        int userResponse = QMessageBox::question(this, tr("Assemblage modifie..."), tr("L'assemblage '%1' a ete modifie.\nContinuer sans enregistrer les modifications ?").arg(currentAssemblyName), QMessageBox::Yes | QMessageBox::No, QMessageBox::No);

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

            int userResponse = QMessageBox::question(this, tr("Modification de parametres..."), tr("Les parametres de la tache '%1' ont ete modifies.\nEnregistrer les modifications avant de continuer ?").arg(currentJobName), QMessageBox::Yes | QMessageBox::No, QMessageBox::Yes);

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

    PreferencesDialog dialog(this, _preferences);
    dialog.setWindowFlags(Qt::Dialog | Qt::FramelessWindowHint);

    /* new preferences are saved if modified */
    if (dialog.exec() == QDialog::Accepted) {
        _server.xmlTool().writeMatissePreferences(PREFERENCES_FILEPATH, *_preferences);
        updateLanguage(_preferences->language());
        updatePreferredDatasetParameters();
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
        QMessageBox::warning(this, tr("Manuel Utilisateur"), tr("Le fichier du manuel utilisateur '%1' n'existe pas")
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

void AssemblyGui::doFoldUnfoldParameters(bool doUnfold, bool isExplicitAction)
{
    if (doUnfold) {
        qDebug() << "Unfolding parameters";
       _parametersDock->show();
    } else {
       qDebug() << "Folding parameters";
       _parametersDock->hide();
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


void AssemblyGui::slot_showApplicationMode(ApplicationMode mode)
{
    qDebug() << "Start application in mode " << mode;
    _activeApplicationMode = mode;
    loadStyleSheet(mode);

    if (mode == APP_CONFIG) {
        PreferencesDialog dialog(this, _preferences);
        dialog.setWindowFlags(Qt::Dialog | Qt::FramelessWindowHint);
        hide();
        /* Si modifications, on enregistre les nouvelles préférences */
        if (dialog.exec() == QDialog::Accepted) {
            _server.xmlTool().writeMatissePreferences(PREFERENCES_FILEPATH, *_preferences);            
            updateLanguage(_preferences->language());
            updatePreferredDatasetParameters();
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
        emit signal_showWelcome();
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

void AssemblyGui::slot_addQGisPointsToMap(QList<QgsPoint> pointsList_p, QString pointsColor_p, QString layerName_p)
{
    _userFormWidget->addQGisPointsToMap(pointsList_p, pointsColor_p, layerName_p);

}
//void AssemblyGui::slot_selectParameters(bool selectedParameters)
//{
//    _ui->_ACT_saveParameters->setEnabled(selectedParameters);
//}


//void AssemblyGui::slot_showAssembly(QModelIndex index)
//{
//    if (_userFormWidget->isVisible()) {
//        return;
//    }
//    if (index.parent().isValid()) {
//        return;
//    }
//    QString filename =  index.sibling(index.row(), 0).data(Qt::UserRole).toString();
//    qDebug() << "Show assembly:" << filename;
//    _expertFormWidget -> loadAssembly(filename);

//}

void AssemblyGui::saveAssemblyAndReload(AssemblyDefinition *assembly)
{
    /* Save assembly and associated parameters */
    QString assemblyName = assembly->name();
    QString filepath = _server.xmlTool().getAssembliesPath() + QDir::separator() + assembly->filename();
    // TODO accès provisoire à la scène par le widget expert
    _expertFormWidget->getScene()->saveAssembly(filepath, assembly);
    //_server.xmlTool().saveAssembly(filepath, assembly);
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

    } else { // Save existing assembly

        QTreeWidgetItem *selectedItem = _ui->_TRW_assemblies->currentItem();
        if (!selectedItem) {
            qCritical() << "Selected item unknown. Assembly cannot be saved";
            return;
        }

        QString assemblyName = getActualAssemblyOrJobName(selectedItem);
        AssemblyDefinition *assembly = _server.xmlTool().getAssembly(assemblyName);
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
        } else {
            qWarning() << "Assembly not found in session repository : " << assemblyName;

            // on désactive la vue graphique
            _expertFormWidget->getScene()->reset();
            _expertFormWidget->getGraphicsView()->setEnabled(false);
        }

    }

    _createAssemblyAct->setEnabled(true);

    // activer / désactiver menus contextuels
    _cloneAssemblyAct->setVisible(true);
    _updateAssemblyPropertiesAct->setVisible(true);
}



void AssemblyGui::displayAssemblyProperties(AssemblyDefinition *selectedAssembly)
{
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

    _assemblyCreationDatePropertyItem->setText(0, tr("Date de création:"));
    _assemblyCreationDatePropertyItem->setText(1, selectedAssembly->date());

    _assemblyAuthorPropertyItem->setText(0, tr("Auteur:"));
    _assemblyAuthorPropertyItem->setText(1, selectedAssembly->author());

    _assemblyCommentPropertyHeaderItem->setText(0, tr("Commentaire:"));

    _assemblyCommentPropertyItemText->setText(selectedAssembly->comment());
    _assemblyCommentPropertyItemText->setToolTip(selectedAssembly->comment()); // to ensure full comment display

    _ui->_TRW_assemblyInfo->addTopLevelItem(_assemblyVersionPropertyItem);
    _ui->_TRW_assemblyInfo->addTopLevelItem(_assemblyCreationDatePropertyItem);
    _ui->_TRW_assemblyInfo->addTopLevelItem(_assemblyAuthorPropertyItem);
    _ui->_TRW_assemblyInfo->addTopLevelItem(_assemblyCommentPropertyHeaderItem);
    _ui->_TRW_assemblyInfo->addTopLevelItem(_assemblyCommentPropertyItem);

    _ui->_TRW_assemblyInfo->header()->resizeSection(0, ASSEMBLY_PROPS_LABEL_WIDTH);
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

    AssemblyDefinition *selectedAssembly = _server.xmlTool().getAssembly(assemblyName);

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
        _currentJob = _server.xmlTool().getJob(jobName);

        if (!_currentJob) {
            qCritical() << QString("No job definition file found for selected job '%1'").arg(jobName);
            return;
        }
    }

    QString assemblyName = _currentJob->assemblyName();
    selectedAssembly = _server.xmlTool().getAssembly(assemblyName);

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
        jobCommentLabelItem->setText(0, tr("Commentaire:"));
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
        jobExecutionDateItem->setText(0, tr("Date d'execution:"));
        QString executionDateStr = _currentJob->executionDefinition()->executionDate().toString(tr("dd/MM/yyyy HH:mm"));
        jobExecutionDateItem->setText(1, executionDateStr);
        _ui->_TRW_assemblyInfo->addTopLevelItem(jobExecutionDateItem);

        jobResultLabelItem = new QTreeWidgetItem();
        jobResultLabelItem->setText(0, tr("Images resultat:"));
        _ui->_TRW_assemblyInfo->addTopLevelItem(jobResultLabelItem);


        QStringList resultImages = _currentJob->executionDefinition()->resultFileNames();
        nbOfImageFiles = resultImages.size();

        jobResultImageItems = new QTreeWidgetItem *[nbOfImageFiles];

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

            // display image
            loadResultToCartoView(resultFile);
        }
    }

    /* resize first column */
    _ui->_TRW_assemblyInfo->header()->resizeSection(0, ASSEMBLY_PROPS_LABEL_WIDTH);
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

    if (promptUser) {
        int ret = QMessageBox::question(this, tr("Supprimer l'assemblage"), tr("Voulez vous supprimer l'assemblage %1 ?").arg(assemblyName), QMessageBox::Ok | QMessageBox::Cancel, QMessageBox::Cancel);
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

    AssemblyDefinition *assembly = _server.xmlTool().getAssembly(assemblyName);
    if (!assembly) {
        qCritical() << QString("Assembly '%1' selected to be removed was not found in local repository").arg(assemblyName);
        return;
    }

    qDebug() << "Removing assembly and assembly parameters files...";

    QString filename = _server.xmlTool().getAssembly(assemblyName)->filename();
    // suppression fichier assemblage
    QString assemblyFilepath = _server.xmlTool().getAssembliesPath() + QDir::separator() + filename;
    QString paramFilepath = _server.xmlTool().getAssembliesParametersPathNew() + QDir::separator() + filename;

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

    AssemblyDefinition * assembly = _server.xmlTool().getAssembly(assemblyName);

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

    JobDialog dialog(this, &kvl, _server.xmlTool().getJobsPath());
    dialog.setWindowFlags(Qt::Dialog | Qt::FramelessWindowHint);
    if (dialog.exec() != QDialog::Accepted) {
        return;
    }

    QString assemblyVersion = assembly->version().simplified();
//    QString parametersVersion = assembly->parametersDefinition()->model();

    QString jobName = kvl.getValue("name");

    JobDefinition newJob(jobName, assemblyName, assemblyVersion);
    newJob.setComment(kvl.getValue("comment"));

    ExecutionDefinition executionDefinition;
    executionDefinition.setExecuted(false);
    newJob.setExecutionDefinition(&executionDefinition);

    if (!_server.xmlTool().writeJobFile(&newJob)) {
        qCritical() << QString("Job definition file could not be created for new job '%1'").arg(jobName);
        return;
    }

    _server.parametersManager()->createJobParametersFile(assemblyName, jobName, kvl);

    // load job definition to local repository
    QString jobFilename = jobName;
    jobFilename.append(".xml");
    _server.xmlTool().readJobFile(jobFilename);
    JobDefinition* newJobDef = _server.xmlTool().getJob(jobName);

    // display and select new job (flag is set to false because job was already saved)
    if (addJobInTree(newJobDef, false)) {
        selectJob(jobName);
    }

    _server.parametersManager()->toggleReadOnlyMode(false); // enable parameters editing
    doFoldUnfoldParameters(true);
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

        _currentJob = _server.xmlTool().getJob(jobName);

        if (!_currentJob) {
            qCritical() << QString("Selected job '%1' not found in local repository").arg(jobName);
            return;
        }
    }

    // modif logique: on peut enregistrer un job deja execute: on enleve l'etat executed et on ecrase...
    if (_currentJob->executionDefinition()->executed()) {
        if (QMessageBox::No == QMessageBox::question(this, tr("Travail execute..."),
                                                     tr("Le travail a deja ete execute.\nVoulez-vous l'ecraser ?"),
                                                     QMessageBox::Yes,
                                                     QMessageBox::No)) {
            return;
        }
        _currentJob->executionDefinition()->setExecuted(false);
        _currentJob->executionDefinition()->setExecutionDate(QDateTime());
        _currentJob->executionDefinition()->setResultFileNames(QStringList() << "");
        // changement etat led + effacement image
        item->setIcon(0, QIcon(":/qss_icons/icons/led-grey.svg"));
        _userFormWidget->clear();
    }


    /* save job and parameters */
    if (!_server.xmlTool().writeJobFile(_currentJob, true)) {
        showError(tr("Fichier de travail..."), tr("Le fichier %1 n'a pu etre ecrit...").arg(_currentJob->name() + ".xml"));
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

    int ret = QMessageBox::question(this, tr("Supprimer le travail"), tr("Voulez vous supprimer le travail %1?").arg(jobName), QMessageBox::Ok | QMessageBox::Cancel, QMessageBox::Cancel);
    if (ret==QMessageBox::Cancel) {
        return;
    }
    QString filename = _server.xmlTool().getJob(jobName)->filename();
    // suppression fichier job

    filename = _server.xmlTool().getJobsPath() + QDir::separator() + filename;
    QFile file(filename);
    if (!file.exists()) {
        qCritical() << QString("Job file '%1' not found, impossible to remove").arg(filename);
        return;
    }

    if (!file.remove()) {
        qCritical() << QString("Job file '%1' could not be removed").arg(filename);
        return;
    }

    filename = _server.xmlTool().getJobsParametersPath(jobName);

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
        contextMenu->addAction(_restoreJobAct);
        contextMenu->addAction(_updateAssemblyPropertiesAct);

    } else {              // menu contextuel pour tâche
        contextMenu->addAction(_executeJobAct);
        contextMenu->addAction(_goToResultsAct);
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
    qDebug() << "Adding to assemblies tree" << assembly;
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

    QTreeWidgetItem * jobItem = new QTreeWidgetItem(assemblyItem, QStringList() << jobName);
    jobItem->setData(0, Qt::UserRole, assembly);
    if (executed) {
        jobItem->setIcon(0, QIcon(":/qss_icons/icons/led-green.svg"));
    } else {
        jobItem->setIcon(0, QIcon(":/qss_icons/icons/led-grey.svg"));
    }

    return jobItem;
}

void AssemblyGui::initStatusBar()
{
//    _statusProgressBar.setRange(0, 1);
//    _statusProgressBar.setValue(0);

    _messagesLevelIcons.insert(IDLE, QIcon(":/qss_icons/icons/led-grey.svg"));
    _messagesLevelIcons.insert(OK, QIcon(":/qss_icons/icons/led-green.svg"));
    _messagesLevelIcons.insert(WARNING, QIcon(":/qss_icons/icons/led-orange.svg"));
    _messagesLevelIcons.insert(ERROR, QIcon(":/qss_icons/icons/led-red.svg"));

    _messagePictoByLevel.insert(IDLE, QPixmap(":/qss_icons/icons/Message-grey.svg"));
    _messagePictoByLevel.insert(OK, QPixmap(":/qss_icons/icons/Message-green.svg"));
    _messagePictoByLevel.insert(WARNING, QPixmap(":/qss_icons/icons/Message-orange.svg"));
    _messagePictoByLevel.insert(ERROR, QPixmap(":/qss_icons/icons/Message-red.svg"));


    // ProgressBar pour etat... a ajouter
    //statusBar()->addWidget(&_statusProgressBar);
//    int comboWidth = qMin(width()-40, width() * 8 / 10);
//    _messagesCombo.setMinimumWidth(comboWidth);
//    _messagesCombo.setMinimumWidth(4*_ui->_TRW_assemblies->width());
    //_messagesCombo.setSizePolicy(QSizePolicy::MinimumExpanding, QSizePolicy::Fixed);
//    _messagesResetButton.setIcon(QIcon(":/png/edit-delete-3.png"));
//    _messagesResetButton.setIconSize(QSize(20,20));
    _statusMessageWidget = new StatusMessageWidget(this);
    _statusMessageWidget->setObjectName("_WID_statusMessage");
    statusBar()->addPermanentWidget(_statusMessageWidget, 10);
    //statusBar()->setAlignment(Qt::AlignVCenter)
    //statusBar()->addPermanentWidget(_statusLed);

    // Recherche du pictogramme de message
    _messagesPicto = findChild<QLabel*>(QString("_LA_messagesPicto"));

    showStatusMessage();
}

void AssemblyGui::showStatusMessage(QString message, MessageIndicatorLevel level, bool progressOn)
{
    if (message.isEmpty()) {
        return;
    }
    _statusMessageWidget->addMessage(message, _messagesLevelIcons.value(level));

    QPixmap messagesPictoIcon = _messagePictoByLevel.value(level);
    _messagesPicto->setPixmap(messagesPictoIcon);

   // _messagesCombo.insertItem(0, QIcon(_messagesIndicators.value(level)), message);
   // _messagesCombo.setCurrentIndex(0);
//    statusBar()->showMessage(message);
//    if (progressOn) {
//        statusBar()->insertWidget(0, &_statusProgressBar);
//    } else {
//        statusBar()->removeWidget(&_statusProgressBar);
//    }
    //_statusLed->setPixmap(QPixmap(_messagesIndicators.value(level)));    
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
    _fileMenu->setTitle(tr("FICHIER"));
    _exportMapViewAct->setText(tr("Exporter la vue carto en image"));
    _exportProjectQGisAct->setText(tr("Exporter le projet en fichier QGIS"));
    _closeAct->setText(tr("Fermer"));

    /* MENU AFFICHAGE */
    _displayMenu->setTitle(tr("AFFICHAGE"));
    _dayNightModeAct->setText(tr("Mode jour/nuit"));
    _mapToolbarAct->setText(tr("Barres d'outils"));

    /* MENU TRAITEMENTS */
    _processMenu->setTitle(tr("TRAITEMENTS"));
    _createAssemblyAct->setText(tr("Creer"));
    _saveAssemblyAct->setText(tr("Enregistrer"));
    _importAssemblyAct->setText(tr("Importer"));
    _exportAssemblyAct->setText(tr("Exporter"));

    /* MENU OUTILS */
    _toolMenu->setTitle(tr("OUTILS"));
    _appConfigAct->setText(tr("Configurer les parametres de l'application"));
    _exposureToolAct->setText(tr("Lancer outil rognage et correction d'illumination"));
    _videoToImageToolAct->setText(tr("Lancer outil transformation de videos en jeux d'image"));
    _checkNetworkRxAct->setText(tr("Verifier reception reseau"));

    /* Sous-menu Cartographie */
    _mapMenu->setTitle(tr("Cartographie"));
    _loadShapefileAct->setText(tr("Charger un shapefile"));
    _loadRasterAct->setText(tr("Charger un raster"));

    /* MENU AIDE */
    _helpMenu->setTitle(tr("AIDE"));
    _userManualAct->setText(tr("Manuel utilisateur"));
    _aboutAct->setText(tr("A propos"));


    /* Menu contextuel Traitement */
    _createJobAct->setText(tr("Creer une nouvelle tache"));
    _importJobAct->setText(tr("Importer une nouvelle tache"));
    _cloneAssemblyAct->setText(tr("Dupliquer"));
    _deleteAssemblyAct->setText(tr("Supprimer la chaine de traitement"));
    _restoreJobAct->setText(tr("Restaurer"));
    _updateAssemblyPropertiesAct->setText(tr("Mettre a jour les proprietes"));

    /* Menu contextuel Tâche */
    _executeJobAct->setText(tr("Executer"));
    _saveJobAct->setText(tr("Enregistrer"));
    _cloneJobAct->setText(tr("Dupliquer"));
    _exportJobAct->setText(tr("Exporter"));
    _deleteJobAct->setText(tr("Supprimer"));
    _archiveJobAct->setText(tr("Archiver"));
    _goToResultsAct->setText(tr("Ouvrir emplacement du resultat"));
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
        _maximizeOrRestoreButton->setIcon(_maximizeIcon);
    } else {
        showMaximized();
        _maximizeOrRestoreButton->setIcon(_restoreToNormalIcon);
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
    QString filename = name + ".xml";
    filename.replace(" ", "_"); // normalization
    _newAssembly->setFilename(filename);

    QTreeWidgetItem *item = addAssemblyInTree(_newAssembly);

    _ui->_TRW_assemblies->setCurrentItem(item);
    item->setSelected(true);

    _currentAssembly = NULL;

    //slot_selectAssemblyOrJob(item);
    displayAssemblyProperties(_newAssembly);

    // unfold parameters
    doFoldUnfoldParameters(true);

    // activer vue graphique
    _expertFormWidget->getGraphicsView()->setEnabled(true);

    // activer / désactiver menus
    _createAssemblyAct->setEnabled(false);
    _saveAssemblyAct->setEnabled(true);

    // activer / désactiver menus contextuels
    _cloneAssemblyAct->setVisible(false);
    _updateAssemblyPropertiesAct->setVisible(false);
}

void AssemblyGui::enableActions()
{
    /* activer/désactiver les items du menu principal */
    _exportMapViewAct->setEnabled(_isMapView);
    _exportProjectQGisAct->setEnabled(_isMapView);
    _mapToolbarAct->setEnabled(_isMapView);
    _exposureToolAct->setEnabled(_isMapView);
    _videoToImageToolAct->setEnabled(_isMapView);
    _loadShapefileAct->setEnabled(_isMapView);
    _loadRasterAct->setEnabled(_isMapView);
    _createAssemblyAct->setEnabled(!_isMapView);
    //_saveAssemblyAct->setEnabled(!_isMapView);
    _saveAssemblyAct->setEnabled(false);
    _checkNetworkRxAct->setEnabled(_isMapView);

    /* montrer/cacher les items de menu contextuel (Traitement) */
    _createJobAct->setVisible(_isMapView);
    // TODO dupliquer & propriétés
    _importJobAct->setVisible(_isMapView);
    _cloneAssemblyAct->setVisible(!_isMapView);
    _updateAssemblyPropertiesAct->setVisible(!_isMapView);
    _restoreJobAct->setVisible(_isMapView);
    _deleteAssemblyAct->setVisible(!_isMapView);

    /* Le menu contextuel tâche est toujours actif, car tâches visibles seulement
     * dans la vue carto */
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
            _activeViewOrModeLabel->setText(tr("Vue : Creation"));
        }
        _visuModeButton->setIcon(_creationVisuModeIcon);

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
            _activeViewOrModeLabel->setText(tr("Vue : Cartographie"));
        }
        _visuModeButton->setIcon(_mapVisuModeIcon);

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

    enableActions();

}

void AssemblyGui::slot_launchJob()
{
    qDebug() << "Launching job...";


    // On test assemblage ou job
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

        _currentJob = _server.xmlTool().getJob(jobName);

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
        if (QMessageBox::No == QMessageBox::question(this, tr("Travail execute..."),
                                                     tr("Le travail a deja ete execute.\nVoulez-vous le relancer?"),
                                                     QMessageBox::Yes,
                                                     QMessageBox::No)) {
            return;
        }
        _userFormWidget->clear();
    }

    // If a parameter was modified, the user is prompted for saving parameter values
    if (_jobParameterModified) {
        if (QMessageBox::No == QMessageBox::question(this, tr("Modification de parametres..."),
                                                     tr("Un ou plusieurs parametres ont ete modifies.\nVoulez-vous enregistrer le parametrage de la tache ?"),
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

    AssemblyDefinition * assemblyDef = _server.xmlTool().getAssembly(assemblyName);
    if (!assemblyDef) {
        qCritical() << "Erreur recup assemblage" << assemblyName;
        showStatusMessage(tr("Erreur sur l'assemblage"), ERROR);
        return;
    }

    QString jobFilename = _currentJob->filename();

    _userFormWidget->switchCartoViewTo(QImageView);

    qDebug() << "Execute le job " << jobName;

    QString msg1=tr("Fichier introuvable.");
    QString msg2=tr("Le fichier de travail %1 n'a pu etre lance").arg(jobName);

    // on recharge le fichier...
    if (!_server.xmlTool().readJobFile(jobFilename)) {
        qCritical() << QString("Error reading job file for job '%1' before launch").arg(jobName);
        QMessageBox::information(this, msg1, msg2);
        showStatusMessage(msg1+ " " + msg2, ERROR);
        return;
    }

    _currentJob = _server.xmlTool().getJob(jobName);

    if (!_currentJob) {
        qCritical() << QString("Job '%1' could not be loaded properly before launch").arg(jobName);
        QMessageBox::information(this, msg1, msg2);
        showStatusMessage(msg1+ " " + msg2, ERROR);
        return;
    }

    _lastJobLaunchedItem = currentItem;
    _lastJobLaunchedItem->setIcon(0, QIcon(":/qss_icons/icons/led-orange.svg"));
    QString msg = tr("Travail %1 en cours...").arg(jobName);
    showStatusMessage(msg, IDLE, true);

    doFoldUnfoldParameters(false);
    _stopButton->setEnabled(true);
    _ongoingProcessInfolabel->show();
    _ongoingProcessCompletion->show();
    emit signal_processRunning();

    bool runSuccess = _server.processJob(*_currentJob);

    if (!runSuccess) {
        QString msg = tr("Erreur %1: %2").arg(jobName).arg(_server.messageStr());
        showStatusMessage(msg, ERROR, true);
        _stopButton->setEnabled(false);
    }else{
        freezeJobUserAction(true);
    }

    //setActionsStates(_lastJobLaunchedItem);
}

void AssemblyGui::slot_stopJob()
{
    QString jobName = _lastJobLaunchedItem->data(0, Qt::UserRole).toString();
    qDebug() << "Arrêt du job " << jobName.toAscii();


    QString msg1=tr("Arret du travail en cours.");
    QString msg2=tr("Voulez vous arreter ou annuler le travail?");
    QMessageBox msgBox;
    msgBox.setText(msg1);
    msgBox.setInformativeText(msg2);
    QPushButton *stopButton = msgBox.addButton(tr("Arret"), QMessageBox::AcceptRole);
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

    if (percentComplete > 100) {
        qWarning() << QString("Invalid process completion percentage value : %1").arg(percentComplete);
        return;
    }

    _ongoingProcessCompletion->setValue(percentComplete);
}


void AssemblyGui::slot_jobProcessed(QString name, bool isCancelled) {
    qDebug() << "Job done : " << name;
    _userFormWidget->switchCartoViewTo(QGisMapLayer);

    if (!_server.errorFlag()) {
        JobDefinition *jobDef = _server.xmlTool().getJob(name);
        QDateTime now = QDateTime::currentDateTime();
        jobDef->executionDefinition()->setExecutionDate(now);
        _server.xmlTool().writeJobFile(jobDef, true);
        if (_lastJobLaunchedItem) {
            if (isCancelled) {
                _lastJobLaunchedItem->setIcon(0, QIcon(":/qss_icons/icons/led-grey.svg"));
                QString msg = tr("Travail %1 annule...").arg(jobDef->name());
                showStatusMessage(msg, OK);
                // TODO Nettoyage image?
            }
            else  {
                // mise a jour de la led...
                _lastJobLaunchedItem->setIcon(0, QIcon(":/qss_icons/icons/led-green.svg"));
                QString msg = tr("Travail %1 termine...").arg(jobDef->name());
                showStatusMessage(msg, OK);

                selectJob(jobDef->name());
            }
        }

        foreach (QString resultFile, jobDef->executionDefinition()->resultFileNames()) {

            if (jobDef->executionDefinition()->executed() && (!resultFile.isEmpty())) {
//                QTreeWidgetItem *item = new QTreeWidgetItem(_ui->_TRW_assemblyInfo, QStringList() << tr("Image resultat:") << resultFile);
//                item->setToolTip(1, resultFile);
                // affichage de l'image
                loadResultToCartoView(resultFile);

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
        _createAssemblyAct->setEnabled(!hasActuallyChanged);
    }
}

void AssemblyGui::slot_modifiedAssembly()
{
    qDebug() << "Received assembly modified notification";

    _isAssemblyModified = true;
    handleAssemblyModified();
    _createAssemblyAct->setEnabled(false);
}

void AssemblyGui::slot_assemblyComplete(bool isComplete)
{
    qDebug() << "Received assembly completeness flag : " << isComplete;

    _isAssemblyComplete = isComplete;
    _saveAssemblyAct->setEnabled(isComplete); // assembly can be saved only if assembly is complete
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


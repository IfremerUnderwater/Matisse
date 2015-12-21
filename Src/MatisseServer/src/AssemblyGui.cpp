#include "AssemblyGui.h"
#include "ui_AssemblyGui.h"

using namespace MatisseServer;

const QString AssemblyGui::PREFERENCES_FILEPATH = QString("config/MatissePreferences.xml");

AssemblyGui::AssemblyGui(QString settingsFile, QWidget *parent) :

    QMainWindow(parent),
    _ui(new Ui::AssemblyGui),
    _mapVisuModeIcon(":/qss_icons/icons/Cartographie.svg"),
    _creationVisuModeIcon(":/qss_icons/icons/Clef.svg"),
    _maximizeIcon(":/qss_icons/icons/agrandir.svg"),
    _restoreToNormalIcon(":/qss_icons/icons/reinittaille.svg")
{
    _ui->setupUi(this);
    _canShow = setSettingsFile(settingsFile);
    init();
    test();

    // TODO : Provisoire, à supprimer une fois la barre d'outils supprimée (actions recablées)
    this->_ui->_TOO_actions->hide();
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


    _parameters = new Tools();
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

    // Recherche des libellés ou pictogrammes
    _activeViewOrModeLabel = findChild<QLabel*>(QString("_LA_activeView"));
    _currentDateTimeLabel = findChild<QLabel*>(QString("_LA_currentDateTime"));

    // Initialisation du menu principal
    initMainMenu();

    // Initialisation des menus contextuels
    initContextMenus();


    // Recherche autres widgets
    HomeWidget* homeWidget = findChild<HomeWidget*>(QString("homeWidget"));

    _userParameterModified = false;
    _expertValuesModified = false;

    _userFormWidget = _ui->_WID_userFrom;
    connect(_userFormWidget, SIGNAL(signal_parametersChanged(bool)), this, SLOT(slot_modifiedParameters(bool)));
    _userFormWidget->setTools(_parameters);

    //connect(_ui->_TRW_assemblies, SIGNAL(doubleClicked(QModelIndex)), this, SLOT(slot_showAssembly(QModelIndex)));
    connect(_ui->_TRW_assemblies, SIGNAL(itemClicked(QTreeWidgetItem*,int)), this, SLOT(slot_selectAssemblyOrJob(QTreeWidgetItem*,int)));
    connect(_ui->_TRW_assemblies, SIGNAL(customContextMenuRequested(QPoint)), this, SLOT(slot_assemblyContextMenuRequested(QPoint)));

    //connect(_ui->_ACT_deleteAssembly, SIGNAL(triggered()), this, SLOT(slot_deleteAssembly()));
    //OK connect(_ui->_ACT_newAssembly, SIGNAL(triggered()), this, SLOT(slot_clearAssembly()));
    //OK connect(_ui->_ACT_saveAssembly, SIGNAL(triggered()), this, SLOT(slot_saveAssembly()));
    //REM connect(_ui->_ACT_saveAsAssembly, SIGNAL(triggered()), this, SLOT(slot_saveAsAssembly()));

    //??? connect(_ui->_ACT_saveParameters, SIGNAL(triggered()), this, SLOT(slot_saveParameters()));
    //??? connect(_ui->_ACT_deleteParameters, SIGNAL(triggered()), this, SLOT(slot_deleteParameters()));

    //KO connect(_ui->_ACT_saveJob, SIGNAL(triggered()), this, SLOT(slot_saveJob()));
    //KO connect(_ui->_ACT_saveAsJob, SIGNAL(triggered()), this, SLOT(slot_saveAsJob()));
    //KO connect(_ui->_ACT_deleteJob, SIGNAL(triggered()), this, SLOT(slot_deleteJob()));
    //OK connect(_ui->_ACT_launchJob, SIGNAL(triggered()), this, SLOT(slot_launchJob()));
    //OK connect(_ui->_ACT_stopJob, SIGNAL(triggered()), this, SLOT(slot_stopJob()));

    //connect(_ui->_ACT_userExpert, SIGNAL(triggered()), this, SLOT(slot_swapUserOrExpert()));
    connect(_visuModeButton, SIGNAL(clicked()), this, SLOT(slot_swapMapOrCreationView()));
    //SUPPR connect(_ui->_ACT_reloadAssembliesAndJobs, SIGNAL(triggered()), this, SLOT(slot_assembliesReload()));

    // Home action
    connect(homeWidget, SIGNAL(signal_goHome()), this, SLOT(slot_goHome()));

    // System actions
    connect(closeButton, SIGNAL(clicked()), this, SLOT(close()));
    connect(_maximizeOrRestoreButton, SIGNAL(clicked()), this, SLOT(slot_maximizeOrRestore()));
    connect(minimizeButton, SIGNAL(clicked()), this, SLOT(showMinimized()));
    connect(_ui->_MCB_controllBar, SIGNAL(signal_moveWindow(QPoint)), this, SLOT(slot_moveWindow(QPoint)));

    // Menu actions
    connect(_closeAct, SIGNAL(triggered()), this, SLOT(close()));
    connect(_createAssemblyAct, SIGNAL(triggered()), this, SLOT(slot_newAssembly()));
    connect(_saveAssemblyAct, SIGNAL(triggered()), this, SLOT(slot_saveAssembly()));
    connect(_updateAssemblyPropertiesAct, SIGNAL(triggered()), this, SLOT(slot_saveAssembly()));
    connect(_appConfigAct, SIGNAL(triggered()), this, SLOT(slot_updatePreferences()));

    // Menus contextuels
    connect(_createJobAct, SIGNAL(triggered()), this, SLOT(slot_newJob()));
    connect(_deleteAssemblyAct, SIGNAL(triggered()), this, SLOT(slot_deleteAssembly()));

    connect(_executeJobAct, SIGNAL(triggered()), this, SLOT(slot_launchJob()));
    connect(_deleteJobAct, SIGNAL(triggered()), this, SLOT(slot_deleteJob()));

    // Tool button actions
    connect(_stopButton, SIGNAL(clicked()), this, SLOT(slot_stopJob()));


    _server.setSettingsFile();
    _server.init();

    _beforeSelect = true;
    _lastJobLaunchedItem = NULL;

    connect(&_server, SIGNAL(signal_jobProcessed(QString, bool)), this, SLOT(slot_jobProcessed(QString, bool)));
    connect(&_server, SIGNAL(signal_jobIntermediateResult(QString,Image *)), this, SLOT(slot_jobIntermediateResult(QString,Image *)));

    initPreferences();

    //loadAssembliesAndJobsLists();

    _expertFormWidget = new ExpertFormWidget(&_server);
    _ui->_SPL_userExpert->insertWidget(1, _expertFormWidget);
    _expertFormWidget->sizePolicy().setHorizontalStretch(_userFormWidget->sizePolicy().horizontalStretch());
    _expertFormWidget->hide();
    _expertFormWidget->getScene()->setMainGui(this);
    //_expertFormWidget->getScene()->setEnabled(false);
    connect(_expertFormWidget, SIGNAL(signal_parametersValuesModified(bool)), this, SLOT(slot_modifiedParameters(bool)));
    connect(_expertFormWidget, SIGNAL(signal_saveParameters()), this, SLOT(slot_saveParameters()));
    connect(_expertFormWidget, SIGNAL(signal_selectParameters(bool)), this, SLOT(slot_selectParameters(bool)));
    connect(_expertFormWidget, SIGNAL(signal_usedParameters(bool)), this, SLOT(slot_usedParameters(bool)));

    // Loading processors
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

    _isMapView = false; // initialisé à false (création) pour être basculé à true (carto) lors du 1er swap
    slot_swapMapOrCreationView();
    _userFormWidget->createCanvas();

    // etat intial actions
    setActionsStates();
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
    _cloneJobAct = new QAction(this);
    _exportJobAct = new QAction(this);
    _deleteJobAct = new QAction(this);
    _archiveJobAct = new QAction(this);
    _goToResultsAct = new QAction(this);

    _ui->_TRW_assemblies->setContextMenuPolicy(Qt::CustomContextMenu);
}

void AssemblyGui::test()
{
    // Pour test
    // Lecture fichier dim2
    // Dim2FileReader reader("C:/WorkspaceMatisse/Test_dataset/otus.dim2");

}

// TODO Chargement des jobs à déplacer dans Server ou Xml
void AssemblyGui::loadAssembliesAndJobsLists(bool doExpand)
{
    // parse du repertoire et recuperation des differents jobs
    //_assembliesValues.clear();
    _currentJobName="";
    QString jobsPath =  _server.xmlTool().getJobsPath();
    QString assembliesPath = _server.xmlTool().getAssembliesPath();

    qDebug() << "Load assemblies from " + assembliesPath;
    QDir assembliesDir(assembliesPath);
    QStringList assemblies = assembliesDir.entryList(QStringList() << "*.xml");
    _server.xmlTool().clearAssembliesDatas();

    // La liste des topItems
    _ui->_TRW_assemblies->clear();
    _assembliesItems.clear();
    _assembliesProperties.clear();

    // chargement des assemblies
    foreach(QString assembly, assemblies) {
        qDebug() << "Lecture assembly" << assembly;
        if (!_server.xmlTool().readAssemblyFile(assembly)) {
            qDebug() << "Unable to read assembly " << assembly;
            continue;
        }
    }

    // deuxième passe, création des items...
    // on met le nom du fichier dans le userRole afin
    // de le rendre accessible ailleurs...
    foreach(QString assemblyName, _server.xmlTool().getAssembliesList()) {
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

    /* activation du bouton de sélection de vue selon mode applicatif */
    bool hasExpertFeatures = (mode == PROGRAMMING);
    bool hasRealTimeFeatures = (mode == PROGRAMMING || mode == REAL_TIME);

    _visuModeButton->setVisible(hasExpertFeatures);
    _createAssemblyAct->setVisible(hasExpertFeatures);
    _saveAssemblyAct->setVisible(hasExpertFeatures);

    _checkNetworkRxAct->setVisible(hasRealTimeFeatures);

    // affichage du mode pour TR/TD (sinon affichage de la vue)
    if (mode == REAL_TIME) {
        _activeViewOrModeLabel->setText(tr("Mode : Temps réel"));
    } else if(mode == DEFERRED_TIME){
        _activeViewOrModeLabel->setText(tr("Mode : Dépouillement"));
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

//bool AssemblyGui::getAssemblyValues(QString fileWithPath, QString name, bool &valid, KeyValueList &assemblyValues)
//{
//    QFile assemblyFile(fileWithPath);
//    QTextStream is(&assemblyFile);
//    if (!assemblyFile.open(QIODevice::ReadOnly)) {
//        qDebug() << "Erreur ouverture lecture" << fileWithPath;
//        return false;
//    }

//    // recherche des valeurs
//    int processorCount = 0;
//    int posStart =-1;
//    int posEnd = -1;
//    QString eltName = "";
//    while (!is.atEnd()) {
//        QString line = is.readLine().trimmed();
//        if  (line.startsWith("<MatisseAssembly ")) {
//            posStart = line.indexOf("name=\"") + 6;
//            posEnd = line.indexOf("\"", posStart);
//            name = line.mid(posStart, posEnd - posStart).trimmed();
//            if (name.isEmpty()) {
//                assemblyFile.close();
//                return false;
//            }

//            posStart = line.indexOf("isRealTime=\"") + 12;
//            posEnd = line.indexOf("\"", posStart);
//            QString realTimeStr = line.mid(posStart, posEnd - posStart).trimmed();
//            bool realTime = QVariant(realTimeStr).toBool();
//            assemblyValues.set("RealTime", realTime);

//            posStart = line.indexOf("valid=\"") + 7;
//            posEnd = line.indexOf("\"", posStart);
//            valid = line.mid(posStart, posEnd - posStart).trimmed().toShort();
//            qDebug() << "Valid=" << valid;

//        } else if (line == "<Version>") {
//            line = is.readLine().trimmed();
//            assemblyValues.set("Version", line);
//            qDebug() << "Version=" << line;
//        } else if (line == "<Date>") {
//            line = is.readLine().trimmed();
//            qDebug() << "Date=" << line;
//            assemblyValues.set("Date", line);
//        } else if (line == "<Author>") {
//            line = is.readLine().trimmed();
//            qDebug() << "Author=" << line;
//            assemblyValues.set(tr("Auteur"), line);
//        } else if (line == "<Comments>") {
//            QString buffer;
//            while ((line = is.readLine().trimmed()) != "</Comments>") {
//                buffer.append(line + "\n");
//            }
//            buffer.chop(1);
//            qDebug() << "Comments=" << buffer;
//            assemblyValues.set(tr("Commentaires"), buffer);
//        } else if (line.startsWith("<Parameters ")) {
//            posStart = line.indexOf("name=\"") + 6;
//            posEnd = line.indexOf("\"", posStart);
//            eltName = line.mid(posStart, posEnd - posStart).trimmed();
//            assemblyValues.set("Parametres", eltName);
//        } else if (line.startsWith("<Source ")) {
//            posStart = line.indexOf("name=\"") + 6;
//            posEnd = line.indexOf("\"", posStart);
//            eltName = line.mid(posStart, posEnd - posStart).trimmed();
//            assemblyValues.set("Source", eltName);
//        } else if (line.startsWith("<Processor ")) {
//            processorCount++;
//            posStart = line.indexOf("name=\"") + 6;
//            posEnd = line.indexOf("\"", posStart);
//            eltName = line.mid(posStart, posEnd - posStart).trimmed();
//            assemblyValues.set(tr("Processeur n°%1").arg(processorCount), eltName);
//        } else if (line == "</Processors>") {
//            break;
//        }
//    }
//    assemblyFile.close();

//    return true;
//}

void AssemblyGui::slot_selectAssemblyOrJob(QTreeWidgetItem * selectedItem, int column)
{
    // TODO Voir s'il est nécessaire de filtrer pour ne pas sélectionner sur click droit
    // Si besoin, il faut implémenter le signal clicked au lieu de itemClicked
    QString name = selectedItem->text(column);


    setActionsStates(selectedItem);

    if (!selectedItem->parent()) {
        // On a selectionné un assemblage
        displayAssembly(name);                

        if (!_isMapView) {
            // on active la vue graphique
            _expertFormWidget->getGraphicsView()->setEnabled(true);

            // provisoire : on active ici l'enregistrement
            _saveAssemblyAct->setEnabled(true);
        }
    } else {
        displayJob(name);
    }
}

void AssemblyGui::slot_assemblyElementsCount(int count)
{
    // test du contenu de l'assemblage pour l'etat des boutons save et save as...
    // on est en mode expert...
    // A unifier avec l'etat des boutons...
//    bool elements = (count > 0);
//    bool noJobs = false;
//    if (_ui->_TRW_assemblies->currentItem()) {
//        noJobs =(_ui->_TRW_assemblies->currentItem()->childCount() == 0);
//    }
//    _ui->_ACT_deleteAssembly->setEnabled(elements && noJobs);
//    _ui->_ACT_saveAssembly->setEnabled(elements && noJobs);
//    _ui->_ACT_saveAsAssembly->setEnabled(elements);
}

void AssemblyGui::slot_saveParameters()
{
    if (_isMapView) {
        return;
    }

    if (_expertFormWidget->saveParameters()) {
        // on recharge les parametres
        _expertFormWidget->selectLastUsedParameter();
        _expertValuesModified = false;
        _ui->_ACT_saveAssembly->setEnabled(true);
        _ui->_ACT_saveAsAssembly->setEnabled(true);
    }
}

void AssemblyGui::slot_deleteParameters()
{
    qDebug() << "Delete parameters";
    if (_isMapView) {
        return;
    }

    int ret = QMessageBox::question(this, tr("Suppression de jeu de parametres"), tr("Voulez vous supprimer le jeu de parametres selectionne?"), QMessageBox::Ok | QMessageBox::Cancel, QMessageBox::Cancel);
    if (ret==QMessageBox::Cancel) {
        return;
    }


    if (_expertFormWidget->deleteSelectedParameters()) {
        _expertFormWidget->selectLastUsedParameter();
        _ui->_ACT_deleteParameters->setEnabled(false);

    }
}

void AssemblyGui::slot_usedParameters(bool usedParameters)
{
    _ui->_ACT_deleteParameters->setEnabled(!usedParameters);
}

void AssemblyGui::slot_updateTimeDisplay()
{
    QDateTime current = QDateTime::currentDateTime();
    _currentDateTimeLabel->setText(current.toString("dd/MM/yyyy hh:mm"));
}

void AssemblyGui::slot_updatePreferences()
{
    PreferencesDialog dialog(this, _preferences);

    /* Si modifications, on enregistre les nouvelles préférences */
    if (dialog.exec() == QDialog::Accepted) {
        _server.xmlTool().writeMatissePreferences(PREFERENCES_FILEPATH, *_preferences);
        updateLanguage(_preferences->language());
    }
}

void AssemblyGui::slot_showApplicationMode(ApplicationMode mode)
{
    qDebug() << "Start application in mode " << mode;
    _activeApplicationMode = mode;
    loadStyleSheet(mode);

    if (mode == APP_CONFIG) {
        PreferencesDialog dialog(this, _preferences);
        hide();
        /* Si modifications, on enregistre les nouvelles préférences */
        if (dialog.exec() == QDialog::Accepted) {
            _server.xmlTool().writeMatissePreferences(PREFERENCES_FILEPATH, *_preferences);            
            updateLanguage(_preferences->language());
        }

        // retour à l'écran d'accueil
        slot_goHome();
    } else {
        slot_clearAssembly();
        _ui->_TRW_assemblies->clearSelection();
        _ui->_TRW_assemblyInfo->clear();
        // On recharge pour n'afficher que les traitements utilisables pour le mode
        slot_assembliesReload();
        show();

        // TODO : à supprimer test GGIS
        //_userFormWidget->loadTestVectorLayer();
        //_userFormWidget->loadShapefile("world_borders.shp");
        //_userFormWidget->loadRasterFile("Blended_Mosaic_stretched.tif");
        //_userFormWidget->showQGisCanvas(true);
    //    _userFormWidget->saveQgisProject("matisse.qgs");
    }

}

void AssemblyGui::slot_goHome()
{
    hide();
    emit signal_showWelcome();
}

void AssemblyGui::slot_selectParameters(bool selectedParameters)
{
    _ui->_ACT_saveParameters->setEnabled(selectedParameters);
}


void AssemblyGui::slot_showAssembly(QModelIndex index)
{
    if (_userFormWidget->isVisible()) {
        return;
    }
    if (index.parent().isValid()) {
        return;
    }
    QString filename =  index.sibling(index.row(), 0).data(Qt::UserRole).toString();
    qDebug() << "Show assembly:" << filename;
    _expertFormWidget -> loadAssembly(filename);

}

void AssemblyGui::saveAssemblyAndReload(AssemblyDefinition *assembly)
{
    QString assemblyName = assembly->name();

    QString filepath = _server.xmlTool().getAssembliesPath() + QDir::separator() + assembly->filename();
    // TODO accès provisoire à la scène par le widget expert
    _expertFormWidget->getScene()->saveAssembly(filepath, assembly);
    slot_assembliesReload();

    // TODO est-ce que ça sert vraiment ?
    displayAssembly(assemblyName);

    // re-sélection de l'assemblage enregistré
    QTreeWidgetItem *item = _assembliesItems.value(assemblyName);
    if (item) {
        _ui->_TRW_assemblies->setCurrentItem(item);
        item->setSelected(true);
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
        // On retire le caractère * à la fin du nom
        QString assemblyName = _newAssembly->name();
        assemblyName.chop(1);
        _newAssembly->setName(assemblyName);

        KeyValueList props = _assembliesProperties.value(assemblyName);

        AssemblyDialog dialog(this, assemblyName, props, false, true);
        if (dialog.exec() != QDialog::Accepted) {
            return;
        }

        _newAssembly->setName(assemblyName);
        _newAssembly->setIsRealTime(QVariant(props.getValue("RealTime")).toBool());
        _newAssembly->setVersion(props.getValue("Version"));
        _newAssembly->setUsable(props.getValue("Valid").toInt());
        _newAssembly->setComment(props.getValue("Comments"));

        saveAssemblyAndReload(_newAssembly);
        _newAssembly->deleteLater();
        _newAssembly = NULL;
    } else {
        QString name = _ui->_TRW_assemblies->currentItem()->text(0);
        AssemblyDefinition *assembly = _server.xmlTool().getAssembly(name);
        if (assembly) {                        
            // mise à jour des propriétés
            KeyValueList props = _assembliesProperties.value(name);

            AssemblyDialog dialog(this, name, props, false, false);
            if (dialog.exec() != QDialog::Accepted) {
                return;
            }

            assembly->setIsRealTime(QVariant(props.getValue("RealTime")).toBool());
            assembly->setVersion(props.getValue("Version"));
            assembly->setUsable(props.getValue("Valid").toInt());
            assembly->setComment(props.getValue("Comments"));

            saveAssemblyAndReload(assembly);
        } else {
            qWarning() << "Assembly not found in session repository : " << name;

            // on désactive la vue graphique
            _expertFormWidget->getScene()->reset();
            _expertFormWidget->getGraphicsView()->setEnabled(false);
        }

    }


//    // au cas où....
//    if (!_isMapView) {
//        qDebug() << "Save assembly";
//        QString name = _ui->_TRW_assemblies->currentItem()->text(0);
//        KeyValueList fields;

//        _expertFormWidget -> saveAssembly(name, fields);
//        slot_assembliesReload();
//        // puis on selectionne...
//        displayAssembly(name);
//    }

    _createAssemblyAct->setEnabled(true);

    // activer / désactiver menus contextuels
    _cloneAssemblyAct->setVisible(true);
    _updateAssemblyPropertiesAct->setVisible(true);
}

void AssemblyGui::slot_saveAsAssembly()
{
    // au cas où....
    if (!_isMapView) {
        qDebug() << "Save as assembly";
        QString name;
        KeyValueList fields;

        AssemblyDialog dialog(this, name, fields, false);
        if (dialog.exec() != QDialog::Accepted) {
            return;
        }

        _expertFormWidget -> saveAssembly(name, fields);
        // ajout dans la liste des assemblages
        // a modifier...
        // on recharge
        slot_assembliesReload();
        // puis on selectionne...
        displayAssembly(name);

    }
}


void AssemblyGui::displayAssemblyProperties(AssemblyDefinition *selectedAssembly)
{
    new QTreeWidgetItem(_ui->_TRW_assemblyInfo, QStringList() << tr("Version:") << selectedAssembly->version());
    new QTreeWidgetItem(_ui->_TRW_assemblyInfo, QStringList() << tr("Date de creation:") << selectedAssembly->date());
    new QTreeWidgetItem(_ui->_TRW_assemblyInfo, QStringList() << tr("Auteur:") << selectedAssembly->author());
    new QTreeWidgetItem(_ui->_TRW_assemblyInfo, QStringList() << tr("Commentaire:") << selectedAssembly->comment());
}

void AssemblyGui::displayAssembly(QString assemblyName) {

    _ui->_TRW_assemblyInfo->clear();
    if(_userFormWidget) {
        _userFormWidget->clear();
    }

    AssemblyDefinition *selectedAssembly = _server.xmlTool().getAssembly(assemblyName);

    if (selectedAssembly) {
        QString parametersVersion = selectedAssembly->parametersDefinition()->model();
        QString modelFile = _server.xmlTool().getModelPath(parametersVersion);
        QString parametersFile = _server.xmlTool().getAssembliesParametersPath(parametersVersion,selectedAssembly->parametersDefinition()->name());
        if (_parameters->readUserParametersFile(parametersFile, modelFile)) {
            _userFormWidget->showUserParameters(true);
        }
        // chargement des infos

        displayAssemblyProperties(selectedAssembly);

        if (!_isMapView) {
            _expertFormWidget->loadAssembly(assemblyName);
            _expertFormWidget->showParameters(selectedAssembly);
        }
    } else {
        _userFormWidget->showUserParameters(false);        
    }
}

void AssemblyGui::displayJob(QString jobName)
{
    _ui->_TRW_assemblyInfo->clear();
    if(_userFormWidget) {
        _userFormWidget->clear();
    }

    JobDefinition * selectedJob = _server.xmlTool().getJob(jobName);
    if (selectedJob) {
        _currentJobName = selectedJob->name();

        QString parametersVersion = selectedJob->parametersDefinition()->model();
        QString modelFile = _server.xmlTool().getModelPath(parametersVersion);

        QString parametersFile = _server.xmlTool().getJobsParametersPath(parametersVersion, selectedJob->parametersDefinition()->name());
        if (_parameters->readUserParametersFile(parametersFile, modelFile)) {
            _userFormWidget->showUserParameters(true);
        }

        // chargement des infos
        QString comments = selectedJob->comment();

        new QTreeWidgetItem(_ui->_TRW_assemblyInfo, QStringList() << tr("Commentaire:") << comments);

        if (selectedJob->executionDefinition() && selectedJob->executionDefinition()->executed()) {
            new QTreeWidgetItem(_ui->_TRW_assemblyInfo, QStringList() << tr("Date d'execution:") << selectedJob->executionDefinition()->executionDate().toString(tr("le dd/MM/yyyy à HH:mm")));

            _userFormWidget->clear();
            foreach (QString resultFile, selectedJob->executionDefinition()->resultFileNames()) {

                if (selectedJob->executionDefinition()->executed() && (!resultFile.isEmpty())) {
                    QTreeWidgetItem *item = new QTreeWidgetItem(_ui->_TRW_assemblyInfo, QStringList() << tr("Image resultat:") << resultFile);
                    item->setToolTip(1, resultFile);
                    // affichage de l'image
                    QFileInfo infoImage(resultFile);
                    if (infoImage.isRelative()) {
                        infoImage.setFile(QDir(_dataPath), resultFile);
                    }
                    if (!infoImage.exists()) {
                        qDebug() << "Erreur fichier image introuvable" << infoImage.absoluteFilePath();
                    }
                    _userFormWidget->loadRasterFile(infoImage.absoluteFilePath());
                    //_userFormWidget->loadRasterFile("C:\\Users\\d.amar\\Desktop\\ImageCiotat_georef.tif");
                    //_userFormWidget->loadRasterFile("C:\\Users\\d.amar\\Desktop\\ImageSicié_georef.tif");


                }
            }
        }
    }
    else {
        _currentJobName = "";
    }
}

void AssemblyGui::selectJob(QString jobName)
{
    bool found=false;
    // on parcourt l'arbre à  la recherche du job
    for (int indexAssembly = 0; indexAssembly < _ui->_TRW_assemblies->topLevelItemCount(); indexAssembly++) {
        QTreeWidgetItem * assemblyItem = _ui->_TRW_assemblies->topLevelItem(indexAssembly);
        for (int indexJob = 0; indexJob < assemblyItem->childCount(); indexJob++) {
            QTreeWidgetItem * jobItem = assemblyItem->child(indexJob);
            if (jobItem->text(0) == jobName) {
                // selection de l'item...
                found = true;
                _ui->_TRW_assemblies->setCurrentItem(jobItem);
                displayJob(jobName);
                break;
            }
        }
        if (found)
            break;
    }
}

void AssemblyGui::slot_deleteAssembly()
{
    // au cas où bis...
    if (_isMapView) {
        return;
    }
    qDebug() << "Delete assembly";
    QTreeWidgetItem * selectedItem = _ui->_TRW_assemblies->currentItem();
    if (!selectedItem) {
        // rien de sélectionné...
        return;
    }

    // TODO : à gérer au niveau du menu contextuel
    if (selectedItem->childCount() > 0) {
        // des jobs existent... impossible de supprimer
        // traité ailleurs normalement...
        return;
    }

    QString assemblyName = selectedItem->text(0);
    int ret = QMessageBox::question(this, tr("Supprimer l'assemblage"), tr("Voulez vous supprimer l'assemblage %1 ?").arg(assemblyName), QMessageBox::Ok | QMessageBox::Cancel, QMessageBox::Cancel);
    if (ret==QMessageBox::Cancel) {
        return;
    }

    if (_newAssembly) {
        if (_newAssembly->name() == assemblyName) {
            // Si on recharge, le nouvel assemblage (pas encore enregistré) disparaît
            qDebug() << QString("Removing new assembly '%1'").arg(assemblyName);
            _newAssembly->deleteLater();
            _newAssembly = NULL;
            slot_assembliesReload();

            // On réactive le menu Créer
            _createAssemblyAct->setEnabled(true);

            return;
        }

        // On peut supprimer un assemblage existant pendant la création d'un nouveau
    }

    AssemblyDefinition *assembly = _server.xmlTool().getAssembly(assemblyName);
    if (!assembly) {
        qWarning() << QString("Assembly '%1' selected to be removed was not found in local repository").arg(assemblyName);
        return;
    }

    QString filename = _server.xmlTool().getAssembly(assemblyName)->filename();
    // suppression fichier assemblage
    filename = _server.xmlTool().getAssembliesPath() + QDir::separator() + filename;
    QFile file(filename);
    if (!file.exists()) {
        // erreur...
        return;
    }

    if (!file.remove()) {
        // erreur
        return;
    }
    // message effacement OK
    // suppression liste: rechargement...
    slot_assembliesReload();
}

void AssemblyGui::slot_newJob()
{
    QTreeWidgetItem * item = _ui->_TRW_assemblies->currentItem();
    if (!item) {
        qCritical() << "No assembly selected !";
        return;
    }

    QString assemblyName = item->text(0);

    AssemblyDefinition * assembly = _server.xmlTool().getAssembly(assemblyName);
    if (!assembly) {
        if (!_server.xmlTool().readAssemblyFile(assemblyName + ".xml")) {
            showError(tr("Fichier d'assemblage..."), tr("Le fichier %1 n'est pas valide ou est illisible...").arg(assemblyName + ".xml") );
            return;
        }
    }

    qDebug() << "Creating new job";
    KeyValueList kvl;
    kvl.append("name");
    kvl.append("comment");
    if (!assembly->isRealTime()) {
        kvl.append("dataPath");
    }
    kvl.set("resultPath", _preferences->defaultResultPath());
    kvl.append("outputFile", _preferences->defaultMosaicFilenamePrefix());
    if (!assembly->isRealTime()) {
        kvl.append("navigationFile");
    }

    JobDialog dialog(this, &kvl, _server.xmlTool().getJobsPath());
    if (dialog.exec() != QDialog::Accepted) {
        return;
    }


    QString assemblyVersion = assembly->version().simplified();
    QString parametersVersion = assembly->parametersDefinition()->model();

    JobDefinition newJob(kvl.getValue("name"), assemblyName, assemblyVersion);
    newJob.setComment(kvl.getValue("comment"));
}

void AssemblyGui::slot_saveJob()
{
    // au cas où...one more time...
    if (!_isMapView) {
        return;
    }

    QTreeWidgetItem * item = _ui->_TRW_assemblies->currentItem();
    if (!item) {
        return;
    }
    qDebug() << "Save job";
    QString jobName = item->text(0);
    JobDefinition * job = _server.xmlTool().getJob(jobName);

    if (!job) {
        return;
    }

    // modif logique: on peut enregistrer un job deja executé: on enleve l'etat executed et on ecrase...
    if (job->executionDefinition()->executed()) {
        if (QMessageBox::No == QMessageBox::question(this, tr("Travail execute..."),
                                                     tr("Le travail a déjà été execute.\nVoulez-vous l'ecraser?"),
                                                     QMessageBox::Yes,
                                                     QMessageBox::No)) {
            return;
        }
        job->executionDefinition()->setExecuted(false);
        job->executionDefinition()->setExecutionDate(QDateTime());
        job->executionDefinition()->setResultFileNames(QStringList() << "");
        // changement etat led + effacement image
        item->setIcon(0, QIcon(":/qss_icons/icons/led-grey.svg"));
        _userFormWidget->clear();
    }


    if (!_server.xmlTool().writeJobFile(job, true)) {
        showError(tr("Fichier de travail..."), tr("Le fichier %1 n'a pu etre ecrit...").arg(job->name() + ".xml"));
        return;
    }
    // enregistrement des parameters
    KeyValueList comments;
    QString parametersVersion = job->parametersDefinition()->model();
    QString parameterFile = _server.xmlTool().getJobsParametersPath(parametersVersion, job->name());
    _parameters->generateParametersFile(parameterFile, parametersVersion, comments);
    // on rechargement lees parameteres afin de valider l'ecriture...
    slot_selectAssemblyOrJob(item);
}

void AssemblyGui::slot_saveAsJob()
{
    // au cas où...one more time...
    if (!_isMapView) {
        return;
    }

    QTreeWidgetItem * item = _ui->_TRW_assemblies->currentItem();
    if (!item) {
        return;
    }
    qDebug() << "Save as job";
    KeyValueList keysValues;
    keysValues.append("name");
    keysValues.append("comment");
    JobDialog dialog(this, &keysValues, _server.xmlTool().getJobsPath());
    if (dialog.exec() != QDialog::Accepted) {
        return;
    }

    QString assemblyName = item->text(0);
    if (item->parent() != NULL){
        assemblyName = item->parent()->text(0);
    }


    AssemblyDefinition * assembly = _server.xmlTool().getAssembly(assemblyName);
    if (!assembly) {
        if (!_server.xmlTool().readAssemblyFile(assemblyName + ".xml")) {
            showError(tr("Fichier d'assemblage..."), tr("Le fichier %1 n'est pas valide ou est illisible...").arg(assemblyName + ".xml") );
            return;
        }
    }
    QString assemblyVersion = assembly->version().simplified();
    QString parametersVersion = assembly->parametersDefinition()->model();

    JobDefinition newJob(keysValues.getValue("name"), assemblyName, assemblyVersion);
    newJob.setComment(keysValues.getValue("comment"));

    ParameterDefinition parameters(parametersVersion, newJob.name());
    newJob.setParametersDefinition(&parameters);
    if (!_server.xmlTool().writeJobFile(&newJob)) {
        showError(tr("Fichier de travail..."), tr("Le fichier %1 n'est pas pu etre ecrit...").arg(newJob.name() + ".xml"));
        return;
    }
    // enregistrement des parameters
    KeyValueList comments;

    QString parameterFile = _server.xmlTool().getJobsParametersPath(parametersVersion, newJob.name());
    _parameters->generateParametersFile(parameterFile, parametersVersion, comments);
    // ajout dans la liste: rechargement
    slot_assembliesReload();
    selectJob(newJob.name());



}

void AssemblyGui::slot_deleteJob()
{
    // au cas où bis...
    if (!_isMapView) {
        return;
    }
    qDebug() << "Delete job";
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


    QString jobName = selectedItem->text(0);
    int ret = QMessageBox::question(this, tr("Supprimer le travail"), tr("Voulez vous supprimer le travail %1?").arg(jobName), QMessageBox::Ok | QMessageBox::Cancel, QMessageBox::Cancel);
    if (ret==QMessageBox::Cancel) {
        return;
    }
    QString filename = _server.xmlTool().getJob(jobName)->filename();
    // suppression fichier job

    filename = _server.xmlTool().getJobsPath() + QDir::separator() + filename;
    QFile file(filename);
    if (!file.exists()) {
        // erreur...
        return;
    }

    if (!file.remove()) {
        // erreur
        return;
    }


    QString modelVersion = _server.xmlTool().getJob(jobName)->parametersDefinition()->model();
    QFile parameterFile(_server.xmlTool().getJobsParametersPath(modelVersion, jobName));

    if (!parameterFile.exists()) {
        // erreur...
        return;
    }

    if (!parameterFile.remove()) {
        // erreur
        return;
    }

    _userFormWidget->resetJobForm();
    // message effacement OK
    // suppression liste: rechargement...
    slot_assembliesReload();

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
        contextMenu->addAction(_deleteAssemblyAct);
        contextMenu->addAction(_restoreJobAct);
        contextMenu->addAction(_updateAssemblyPropertiesAct);

    } else {              // menu contextuel pour tâche
        contextMenu->addAction(_executeJobAct);
        contextMenu->addAction(_goToResultsAct);
        contextMenu->addSeparator();
        contextMenu->addAction(_cloneJobAct);
        contextMenu->addAction(_exportJobAct);
        contextMenu->addAction(_deleteJobAct);
        contextMenu->addAction(_archiveJobAct);
    }

    contextMenu->popup(_ui->_TRW_assemblies->viewport()->mapToGlobal(pos));

}

void AssemblyGui::showError(QString title, QString message) {
    QMessageBox::warning(this, title, message);
}

QTreeWidgetItem *AssemblyGui::addAssemblyInTree(AssemblyDefinition * assembly)
{
    QTreeWidgetItem * assemblyItem = new QTreeWidgetItem(QStringList() << assembly->name());
    assemblyItem->setData(0, Qt::UserRole,assembly->filename());
    _ui->_TRW_assemblies->addTopLevelItem(assemblyItem);
    _assembliesItems.insert(assembly->name(), assemblyItem);

    KeyValueList props;
    props.insert("RealTime", QString::number(assembly->isRealTime()));
    props.insert("Version", assembly->version());
    props.insert("Valid", QString::number(assembly->usable()));
    props.insert("Author", assembly->author());
    props.insert("Comments", assembly->comment());
    _assembliesProperties.insert(assembly->name(), props);

    return assemblyItem;
}

QTreeWidgetItem *AssemblyGui::addJobInTree(JobDefinition * job)
{
    QString assembly = job->assemblyName();
    if (job->executionDefinition() == NULL) {
        qDebug() << "TOTO";
    }
    bool executed = job->executionDefinition()->executed();
    QTreeWidgetItem * assemblyItem = _assembliesItems.value(assembly, NULL);
    if (!assemblyItem) {
        // L'assemblage n'existe pas...
        return NULL;
    }
    QTreeWidgetItem * jobItem = new QTreeWidgetItem(assemblyItem, QStringList() << job->name());
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
    statusBar()->addWidget(_statusMessageWidget);
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

void AssemblyGui::setActionsStates(QTreeWidgetItem *currentItem/*, bool modifiedConfiguration*/)
{
    bool saveStatus = false;
    bool saveAsStatus = false;
    bool deleteStatus = false;
    bool parametersStatus = false;
    bool isProcessingStatus = false;

    //on disable tout pour commencer...
    _ui->_ACT_newAssembly->setEnabled(false);
    _ui->_ACT_saveAssembly->setEnabled(false);
    _ui->_ACT_saveAsAssembly->setEnabled(false);
    _ui->_ACT_deleteAssembly->setEnabled(false);
    _ui->_ACT_saveParameters->setEnabled(false);
    _ui->_ACT_deleteParameters->setEnabled(false);
    _ui->_ACT_saveJob->setEnabled(false);
    _ui->_ACT_saveAsJob->setEnabled(false);
    _ui->_ACT_deleteJob->setEnabled(false);
    _ui->_ACT_launchJob->setEnabled(false);
    _ui->_ACT_stopJob->setEnabled(false);

    if (currentItem) {
        if (_isMapView) {
            if (currentItem->parent()) {
                // on a affaire à un job...
                isProcessingStatus = _server.isProcessingJob();
                saveStatus = !isProcessingStatus;
                saveAsStatus = !isProcessingStatus;
                deleteStatus = !isProcessingStatus;
            } else {
                // on a affaire à un assemblage
                // on ne peut que creer un nouveau job
                saveAsStatus = true;
            }
            _ui->_ACT_saveJob->setEnabled(saveStatus);
            _ui->_ACT_saveAsJob->setEnabled(saveAsStatus);
            _ui->_ACT_deleteJob->setEnabled(deleteStatus);
            _ui->_ACT_launchJob->setEnabled(!isProcessingStatus);
            _ui->_ACT_stopJob->setEnabled(isProcessingStatus);

        } else {
            // mode expert
            saveAsStatus = true;
            parametersStatus = true;
            if (currentItem->childCount() == 0) {
                saveStatus = true;
                deleteStatus = true;
            }

            _ui->_ACT_newAssembly->setEnabled(true);
            _ui->_ACT_saveAssembly->setEnabled(saveStatus);
            _ui->_ACT_saveAsAssembly->setEnabled(saveAsStatus);
            _ui->_ACT_deleteAssembly->setEnabled(deleteStatus);
            _ui->_ACT_saveParameters->setEnabled(parametersStatus);
            _ui->_ACT_deleteParameters->setEnabled(false);
        }
    } else {

        if (!_isMapView) {
         // nouvel assemblage...
            _ui->_ACT_newAssembly->setEnabled(true);
            _ui->_ACT_saveAsAssembly->setEnabled(true);
        }
    }
}

void AssemblyGui::initLanguages()
{
    _serverTranslator_en = new QTranslator();
    _serverTranslator_en->load("i18n/MatisseServer_en");

    _toolsTranslator_en = new QTranslator();
    _toolsTranslator_en->load("i18n/MatisseTools_en");

    // Langue par défaut : Français
    _currentLanguage = "FR";
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

        qApp->installTranslator(_serverTranslator_en);
        qApp->installTranslator(_toolsTranslator_en);
    } else {
        qDebug() << "Restoring UI to French";

        qApp->removeTranslator(_serverTranslator_en);
        qApp->removeTranslator(_toolsTranslator_en);
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
    _createAssemblyAct->setText(tr("Créer"));
    _saveAssemblyAct->setText(tr("Enregistrer"));
    _importAssemblyAct->setText(tr("Importer"));
    _exportAssemblyAct->setText(tr("Exporter"));

    /* MENU OUTILS */
    _toolMenu->setTitle(tr("OUTILS"));
    _appConfigAct->setText(tr("Configurer les paramètres de l'application"));
    _exposureToolAct->setText(tr("Lancer outil rognage et correction d'illumination"));
    _videoToImageToolAct->setText(tr("Lancer outil transformation de vidéos en jeux d'image"));
    _checkNetworkRxAct->setText(tr("Vérifier réception réseau"));

    /* Sous-menu Cartographie */
    _mapMenu->setTitle(tr("Cartographie"));
    _loadShapefileAct->setText(tr("Charger un shapefile"));
    _loadRasterAct->setText(tr("Charger un raster"));

    /* MENU AIDE */
    _helpMenu->setTitle(tr("AIDE"));
    _userManualAct->setText(tr("Manuel utilisateur"));
    _aboutAct->setText(tr("A propos"));


    /* Menu contextuel Traitement */
    _createJobAct->setText(tr("Créer une nouvelle tâche"));
    _importJobAct->setText(tr("Importer une nouvelle tâche"));
    _cloneAssemblyAct->setText(tr("Dupliquer"));
    _deleteAssemblyAct->setText(tr("Supprimer la chaîne de traitement"));
    _restoreJobAct->setText(tr("Restaurer"));
    _updateAssemblyPropertiesAct->setText(tr("Mettre à jour les propriétés"));

    /* Menu contextuel Tâche */
    _executeJobAct->setText(tr("Exécuter"));
    _cloneJobAct->setText(tr("Dupliquer"));
    _exportJobAct->setText(tr("Exporter"));
    _deleteJobAct->setText(tr("Supprimer"));
    _archiveJobAct->setText(tr("Archiver"));
    _goToResultsAct->setText(tr("Ouvrir emplacement du résultat"));
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

    setActionsStates();
}

void AssemblyGui::slot_newAssembly()
{
    slot_clearAssembly();

    QString name;
    KeyValueList fields;

    AssemblyDialog dialog(this, name, fields, true);
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

    // le nom de fichier est dérivé du nom de l'assemblage
    QString filename = name + ".xml";
    filename.replace(" ", "_");
    _newAssembly->setFilename(filename);

    QTreeWidgetItem *item = addAssemblyInTree(_newAssembly);

    _ui->_TRW_assemblies->setCurrentItem(item);
    item->setSelected(true);

    //slot_selectAssemblyOrJob(item);
    displayAssemblyProperties(_newAssembly);

    // activer vue graphique
    _expertFormWidget->getGraphicsView()->setEnabled(true);

    // activer / désactiver menus
    _createAssemblyAct->setEnabled(false);
    _saveAssemblyAct->setEnabled(true);

    // activer / désactiver menus contextuels
    _cloneAssemblyAct->setVisible(false);
    _updateAssemblyPropertiesAct->setVisible(false);
}

void AssemblyGui::setActionStatesNew()
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

    DestinationWidget * newWidget = new DestinationWidget();

    newWidget->clone(wid);

    return newWidget;
}


void AssemblyGui::slot_swapMapOrCreationView()
{
    qDebug() << "Current view displayed : " << ((_isMapView) ? "map" : "creation");

    //if (_ui->_ACT_userExpert->isChecked()) {
    if (_isMapView) {
        qDebug() << "Swapping to creation view";
        _isMapView = false;
        if (_activeApplicationMode == PROGRAMMING) {
            _activeViewOrModeLabel->setText(trUtf8("Vue : Création"));
        }
        _visuModeButton->setIcon(_creationVisuModeIcon);
        _userFormWidget->hide();
        _expertFormWidget->show();
        _ui->_TRW_assemblies->collapseAll();
        _ui->_TRW_assemblies->setItemsExpandable(false);
        slot_clearAssembly();
        // on désactive la vue graphique initialement
        _expertFormWidget->getGraphicsView()->setEnabled(false);
        _ui->_TW_creationViewTabs->setCurrentIndex(0);

    } else {
        //setActionsStates(NULL);
        if (_expertValuesModified) {
            if (QMessageBox::No == QMessageBox::question(this, tr("Assemblages/paramètres modifiés..."), tr("Voulez-vous continuer sans sauvegarder ?"),
                                      QMessageBox::Yes,
                                      QMessageBox::No)) {
                _ui->_ACT_userExpert->setChecked(true);
                return;
            }
            _expertValuesModified = false;
        }
        qDebug() << "Swapping to map view";
        _isMapView = true;
        if (_activeApplicationMode == PROGRAMMING) {
            _activeViewOrModeLabel->setText(tr("Vue : Cartographie"));
        }
        _visuModeButton->setIcon(_mapVisuModeIcon);
        _expertFormWidget->hide();
        _userFormWidget->show();
        // Changement des info bulles
        _ui->_TRW_assemblies->setItemsExpandable(true);
        _ui->_TRW_assemblies->expandAll();
        _userFormWidget->resetJobForm();
        _ui->_TW_mapViewTabs->setCurrentIndex(0);
    }

    _ui->_TRW_assemblies->clearSelection();

    /* select tab set */
    _ui->_SW_helperTabSets->setCurrentIndex(!_isMapView);

    setActionStatesNew();

}

void AssemblyGui::slot_launchJob()
{

    // On test assemblage ou job
    // Le bouton est actif si le job est selectionnable
    // cad si l'image n'a pas été produite...
    // ou si on a selectionné un assemblage
    // dans ce cas, on sauvegarde le job avant de le lancer...

    QTreeWidgetItem * currentItem = _ui->_TRW_assemblies->currentItem();
    QTreeWidgetItem * parentItem = currentItem->parent();

    QString assemblyName = currentItem -> text(0);
    QString jobName = assemblyName;
    KeyValueList jobDesc;

    // test modifs...
    bool saveAs = false;
    if (_userFormWidget->parametersWidget()->hasModifiedValues()) {
        saveAs = true;
    }

    if (parentItem) {
        // on a directement un job
        assemblyName = parentItem -> text(0);
        // Modif: si le job a été lancé, on peut le reexecuter avec warning...
        JobDefinition * job = _server.xmlTool().getJob(jobName);
        if (job->executionDefinition()->executed()) {
            if (QMessageBox::No == QMessageBox::question(this, tr("Travail execute..."),
                                                         tr("Le travail a deja ete execute.\nVoulez-vous le relancer?"),
                                                         QMessageBox::Yes,
                                                         QMessageBox::No)) {
                return;
            }
            _userFormWidget->clear();
        }

        // si un parametre est modifié, on demande si on sauvegarde sous un nouveau nom...
        // INFO: normalement on ne doit plus passer ici suite à la modification de logique des enregistrements...
        if (saveAs) {
            if (QMessageBox::No == QMessageBox::question(this, tr("Modification de parametres..."),
                                                         tr("Un ou plusieurs parametres ont ete modifies.\nVoulez-vous enregistrer le travail sous un autre nom?"),
                                                         QMessageBox::Yes,
                                                         QMessageBox::No)) {
                saveAs = false;
            }
        }
    }
    // on crée un nouveau job à partir d'un assemblage vide: sauvegarde obligatoire:
    // INFO: saveAs n'a plus lieu d'être ici suite modif de la logique des enregistrements...
    if ((!parentItem) || saveAs){
        // on a un assemblage
        // on crée le job...
        // ou on sauvegarde l'assemblage sous un autre nom...
        jobName = JobDialog::newJobName(this, &jobDesc, _server.xmlTool().getJobsPath());
        if (jobName.isEmpty()) {
            showStatusMessage(tr("Travail annulé..."));
            // annulé...
            return;
        }
    }

    AssemblyDefinition * assemblyDef = _server.xmlTool().getAssembly(assemblyName);
    if (!assemblyDef) {
        qDebug() << "Erreur recup assemblage" << assemblyName;
        showStatusMessage(tr("Erreur sur l'assemblage"), ERROR);
        return;
    }

    JobDefinition * job = _server.xmlTool().getJob(jobName);
    QString jobFilename;

    if (!job) {
        // ecriture du fichier de parametres
        QString modelVersion = assemblyDef->parametersDefinition()->model();
        QString parameterFile = _server.xmlTool().getJobsParametersPath(modelVersion, jobDesc.getValue("filename"));

        _parameters->generateParametersFile(parameterFile, modelVersion);

        // nouveau job
        job = new JobDefinition(jobName, assemblyName, assemblyDef->version());
        ParameterDefinition * newParam = new ParameterDefinition(assemblyDef->parametersDefinition()->model(), jobName);
        job->setParametersDefinition(newParam);
        jobFilename = jobDesc.getValue("filename");
        job->setComment(jobDesc.getValue("comment"));
        job->setFilename(jobFilename);
        job->setExecutionDefinition(new ExecutionDefinition());
        if (!_server.xmlTool().writeJobFile(job, true)) {
            QString msg1 = tr("Erreur d'ecriture.");
            QString msg2 = tr("Le fichier de travail %1 n'a pu etre ecrit").arg(jobName);
            QMessageBox::information(this, msg1, msg2);
            showStatusMessage(msg1 + " " + msg2, ERROR);
            return;
        }
        // ajout du job dans la liste et selection
        currentItem = addJobInTree(job);
        if (currentItem) {
            _ui->_TRW_assemblies->sortItems(0, Qt::AscendingOrder);
            currentItem->setSelected(true);
        }


    } else {
        jobFilename = job->filename();
    }

    _userFormWidget->showQGisCanvas(false);

    qDebug() << "Execute le job " << jobName;
    // on recharge le fichier...
    if (!jobName.isEmpty()) {
        if (_server.xmlTool().readJobFile(jobFilename)) {
            JobDefinition *job = _server.xmlTool().getJob(jobName);

            if (job) {
                _lastJobLaunchedItem = currentItem;
                _lastJobLaunchedItem->setIcon(0, QIcon(":/qss_icons/icons/led-orange.svg"));
                QString msg = tr("Travail %1 en cours...").arg(jobName);
                showStatusMessage(msg, IDLE, true);
                _stopButton->setEnabled(true);

                if (!_server.processJob(*job)) {
                    QString msg = tr("Erreur %1: %2").arg(jobName).arg(_server.messageStr());
                    showStatusMessage(msg, ERROR, true);
                    _stopButton->setEnabled(false);
                }
                setActionsStates(_lastJobLaunchedItem);
            } else {
                QString msg1=tr("Fichier introuvable.");
                QString msg2=tr("Le fichier de travail %1 n'a pu etre lance").arg(jobName);
                QMessageBox::information(this, msg1, msg2);
                showStatusMessage(msg1+ " " + msg2, ERROR);

            }
        }
    }

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
    }
    else if (msgBox.clickedButton() == discardButton) {
        _server.stopJob(true);
    }
    else {
        // cancel dialog => do nothing
    }

}

void AssemblyGui::slot_jobIntermediateResult(QString name, Image *image)
{
    if (image) {
        _userFormWidget->displayImage(image);
        delete image;
    }
}

void AssemblyGui::slot_jobProcessed(QString name, bool isCancelled) {
    qDebug() << "Job done : " << name;
    _userFormWidget->showQGisCanvas(true);

    if (!_server.errorFlag()) {
        JobDefinition *jobDef = _server.xmlTool().getJob(name);
        QDateTime now = QDateTime::currentDateTime();
        jobDef->executionDefinition()->setExecutionDate(now);
        _server.xmlTool().writeJobFile(jobDef, true);
        if (_lastJobLaunchedItem) {
            if (isCancelled) {
                _lastJobLaunchedItem->setIcon(0, QIcon(":/qss_icons/icons/led-grey.svg"));
                setActionsStates(_lastJobLaunchedItem);
                QString msg = tr("Travail %1 annule...").arg(jobDef->name());
                showStatusMessage(msg, OK);
                // TODO Nettoyage image?
            }
            else  {
                // mise a jour de la led...
                _lastJobLaunchedItem->setIcon(0, QIcon(":/qss_icons/icons/led-green.svg"));
                setActionsStates(_lastJobLaunchedItem);
                QString msg = tr("Travail %1 termine...").arg(jobDef->name());
                showStatusMessage(msg, OK);

                selectJob(jobDef->name());
            }
        }

        // Désactiver le bouton STOP
        _stopButton->setEnabled(false);
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

    //slot_swapMapOrCreationView();
}

void AssemblyGui::slot_modifiedParameters(bool changed)
{
    QObject * sendObjet = sender();
    if (sendObjet == _userFormWidget) {
        // on disable le bouton de lancement si true et si ce n'est pas un assemblage qui est sélectionné...
        QTreeWidgetItem * currentItem = _ui->_TRW_assemblies->currentItem();
        if (currentItem && (currentItem->parent() == NULL)) {
            changed = true;
        }
        _ui->_ACT_launchJob->setDisabled(changed);
        qDebug() << "Disable bouton slot_modifiedParameters" << changed;
        _userParameterModified = changed;
    } else {
        qDebug() << "expert parameters modified";
        _expertValuesModified = changed;
        // on disable les boutons de sauvegarde pour forcer l'enregistrement ou le rechargement
        // Commenté car NOK après un new assembly...
        //        _ui->_ACT_saveAssembly->setDisabled(true);
        //        _ui->_ACT_saveAsAssembly->setDisabled(true);
    }
}

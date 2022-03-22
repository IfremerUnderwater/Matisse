#include <QStyle>
#include <QDesktopWidget>
#include <QMessageBox>

#include "main_gui.h"
#include "ui_main_gui.h"
#include "matisse_version_widget.h"
#include "visu_mode_widget.h"
#include "ongoing_process_widget.h"
#include "graphical_charter.h"

namespace matisse {

const QString MainGui::PREFERENCES_FILEPATH = QString("config/MatissePreferences.xml");
const QString MainGui::JOB_REMOTE_PREFIX = QString("job_remote_");
const QString MainGui::DEFAULT_RESULT_PATH = QString("./outReconstruction");
const QString MainGui::DEFAULT_MOSAIC_PREFIX = QString("MyProcess");

MainGui::MainGui(QWidget *_parent) :

    QMainWindow(_parent),
    m_ui(new Ui::MainGui),
    m_is_map_view(false), // initialized to false (creation) so it will be swapped to true (map) at init
    m_job_parameter_modified(false),
    m_is_assembly_complete(false),
    m_last_job_launched_item(NULL),
    m_new_assembly(NULL),
    m_current_assembly(NULL),
    m_current_job(NULL),
    m_is_night_display_mode(false),
    m_is_assembly_modified(false),
    m_assembly_version_property_item(NULL),
    m_assembly_creation_date_property_item(NULL),
    m_assembly_author_property_item(NULL),
    m_assembly_comment_property_header_item(NULL),
    m_assembly_comment_property_item(NULL),
    m_assembly_comment_property_item_text(NULL),
    m_stop_button(NULL),
    m_minimize_button(NULL),
    m_maximize_or_restore_button(NULL),
    m_home_button(NULL),
    m_home_widget(NULL),
    m_visu_mode_button(NULL),
    m_reset_messages_button(NULL),
    m_max_or_restore_button_wrapper(NULL),
    m_visu_mode_button_wrapper(NULL),
    m_camera_manager_tool_dialog(this),
    m_engine(NULL)
{
    m_ui->setupUi(this);
    m_engine.setJobLauncher(this);
    this->setWindowFlags(Qt::FramelessWindowHint);

}

MainGui::~MainGui()
{
    delete m_icon_factory;
    delete m_ui;
    //qDebug() << "Delete Gui";
}

void MainGui::initDateTimeDisplay()
{
    sl_updateTimeDisplay();
    m_date_time_timer = new QTimer(this);
    m_date_time_timer->setInterval(1000);
    connect(m_date_time_timer, SIGNAL(timeout()), this, SLOT(sl_updateTimeDisplay()));
    m_date_time_timer->start();
}

void MainGui::updatePreferredDatasetParameters()
{
    KeyValueList kvl;
    kvl.insert(DATASET_PARAM_OUTPUT_DIR, m_preferences->defaultResultPath());
    kvl.insert(DATASET_PARAM_OUTPUT_FILENAME, m_preferences->defaultMosaicFilenamePrefix());

    m_engine.parametersManager()->pushPreferredDatasetParameters(kvl);
}

void MainGui::setAssemblyHelper(AssemblyHelper *_assembly_helper)
{
    m_assembly_helper = _assembly_helper;
}

void MainGui::setJobHelper(JobHelper *_job_helper)
{
    m_job_helper = _job_helper;
}

void MainGui::setImportExportHelper(ImportExportHelper *_import_export_helper)
{
    m_import_export_helper = _import_export_helper;
}

void MainGui::setRemoteJobHelper(RemoteJobHelper *_remote_job_helper)
{
    m_remote_job_helper = _remote_job_helper;
}

void MainGui::initPreferences()
{
    m_preferences = new MatissePreferences();

    QFile prefs_file(PREFERENCES_FILEPATH);

    SystemDataManager* system_data_manager = SystemDataManager::instance();

    if (!prefs_file.exists()) {
        // Creating preferences file
        m_preferences->setLastUpdate(QDateTime::currentDateTime());
        m_preferences->setDefaultResultPath(DEFAULT_RESULT_PATH);
        m_preferences->setDefaultMosaicFilenamePrefix(DEFAULT_MOSAIC_PREFIX);
        m_preferences->setProgrammingModeEnabled(false); // By default, programming mode is disabled
        m_preferences->setLanguage("FR");

        system_data_manager->writeMatissePreferences(PREFERENCES_FILEPATH, *m_preferences);
    } else {
        system_data_manager->readMatissePreferences(PREFERENCES_FILEPATH, *m_preferences);
    }

    updateLanguage(m_preferences->language(), true);

    updatePreferredDatasetParameters();
}

void MainGui::initParametersWidget()
{
    m_parameters_dock = m_ui->_SCA_parametersDock;

    m_engine.parametersManager()->setIconFactory(m_icon_factory);
    m_engine.parametersManager()->generateParametersWidget(m_parameters_dock);
    m_parameters_widget = m_engine.parametersManager()->parametersWidget();

    m_parameters_dock->setWidget(m_parameters_widget);

    connect(m_parameters_widget, SIGNAL(si_valuesModified(bool)), this, SLOT(sl_modifiedParameters(bool)));
}

void MainGui::initProcessorWidgets()
{
    qDebug() << "Available processors " << m_engine.getAvailableProcessors().size();
    foreach (Processor * processor, m_engine.getAvailableProcessors()) {
        qDebug() << "Add processor " << processor->name();
        ProcessorWidget * proc_widget = new ProcessorWidget();
        proc_widget->setName(processor->name());
        proc_widget->setInputsNumber(processor->inNumber());
        proc_widget->setOutputsNumber(processor->outNumber());

        QListWidgetItem * new_proc_item = new QListWidgetItem(proc_widget->getIcon(), proc_widget->getName());
        new_proc_item->setData(Qt::UserRole, qlonglong(proc_widget));
        m_available_processors.insert(proc_widget->getName(), proc_widget);
        m_ui->_LW_processors->addItem(new_proc_item);

        //_expertFormWidget->addProcessorWidget(procWidget);
    }


    qDebug() << "Available ImageProviders " << m_engine.getAvailableInputDataProviders().size();
    foreach (InputDataProvider * image_provider, m_engine.getAvailableInputDataProviders()) {
        qDebug() << "Add imageProvider " << image_provider->name();
        SourceWidget * src_widget = new SourceWidget();
        src_widget->setName(image_provider->name());
        src_widget->setOutputsNumber(image_provider->outNumber());

        QListWidgetItem * new_src_item = new QListWidgetItem(src_widget->getIcon(), src_widget->getName());
        new_src_item->setData(Qt::UserRole, qlonglong(src_widget));
        m_available_sources.insert(src_widget->getName(), src_widget);
        m_ui->_LW_inputs->addItem(new_src_item);

        //_expertFormWidget->addSourceWidget(srcWidget);
    }

    qDebug() << "Available RasterProviders " << m_engine.getAvailableOutputDataWriters().size();
    foreach (OutputDataWriter * raster_provider, m_engine.getAvailableOutputDataWriters()) {
        qDebug() << "Add rasterProvider " << raster_provider->name();
        DestinationWidget * dest_widget = new DestinationWidget();
        dest_widget->setName(raster_provider->name());
        dest_widget->setInputsNumber(raster_provider->inNumber());

        QListWidgetItem * new_dest_item = new QListWidgetItem(dest_widget->getIcon(), dest_widget->getName());
        new_dest_item->setData(Qt::UserRole, qlonglong(dest_widget));
        m_available_destinations.insert(dest_widget->getName(), dest_widget);
        m_ui->_LW_outputs->addItem(new_dest_item);
    }
}

void MainGui::lookupChildWidgets()
{
    // Recherche des boutons d'action et système
    m_visu_mode_button = findChild<QToolButton*>(QString("_TBU_visuModeSwap"));
    m_home_button = findChild<QToolButton*>(QString("_TB_homeButton"));
    m_close_button = findChild<QToolButton*>(QString("_TBU_closeButton"));
    m_maximize_or_restore_button = findChild<QToolButton*>(QString("_TBU_maximizeRestoreButton"));
    m_minimize_button = findChild<QToolButton*>(QString("_TBU_minimizeButton"));
    m_stop_button = findChild<QToolButton*>(QString("_TBU_stopButton"));
    m_reset_messages_button = findChild<QPushButton*>(QString("_PB_resetMessages"));

    // Recherche des libellés, pictogrammes ou indicateurs
    m_active_view_or_mode_label = findChild<QLabel*>(QString("_LA_activeView"));
    m_current_date_time_label = findChild<QLabel*>(QString("_LA_currentDateTime"));
    m_ongoing_process_info_label = findChild<QLabel*>(QString("_LA_ongoingProcessInfoLabel"));
    m_matisse_version_label = findChild<QLabel*>(QString("_LAB_matisseVersion"));
    m_ongoing_process_completion = findChild<QProgressBar*>(QString("_PB_ongoingProcessCompletion"));
    m_live_process_wheel = findChild<LiveProcessWheel*>(QString("_WID_liveProcessWheel"));
    m_messages_picto = findChild<QLabel*>(QString("_LA_messagesPicto"));

    // Recherche autres widgets
    m_home_widget = findChild<HomeWidget*>(QString("homeWidget"));

    // Tabs : object name is set anew explicitely to enable stylesheet ( setObjectName overriden)
    QTabWidget *info_view_tabs = findChild<QTabWidget*>(QString("_TW_infoTabs"));
    info_view_tabs->setObjectName("_TW_infoTabs");
    QTabWidget *creation_view_tabs = findChild<QTabWidget*>(QString("_TW_creationViewTabs"));
    creation_view_tabs->setObjectName("_TW_creationViewTabs");

    m_data_viewer = m_ui->_WID_mapViewSceneContainer;
    m_assembly_editor = m_ui->_WID_creationSceneContainer;
}

void MainGui::initProcessWheelSignalling()
{
    connect(this, SIGNAL(si_processRunning()), m_live_process_wheel, SLOT(sl_processRunning()));
    connect(this, SIGNAL(si_processStopped()), m_live_process_wheel, SLOT(sl_processStopped()));
    connect(this, SIGNAL(si_processFrozen()), m_live_process_wheel, SLOT(sl_processFrozen()));
    connect(this, SIGNAL(si_updateWheelColors(QString)), m_live_process_wheel, SLOT(sl_updateWheelColors(QString)));
}

void MainGui::initUserActions()
{
    /* Assembly tree actions */
    connect(m_ui->_TRW_assemblies, SIGNAL(itemClicked(QTreeWidgetItem*,int)), this, SLOT(sl_selectAssemblyOrJob(QTreeWidgetItem*,int)));
    connect(m_ui->_TRW_assemblies, SIGNAL(customContextMenuRequested(QPoint)), this, SLOT(sl_assemblyContextMenuRequested(QPoint)));

    connect(m_visu_mode_button, SIGNAL(clicked()), this, SLOT(sl_swapMapOrCreationView()));

    // Home action
    connect(m_home_widget, SIGNAL(si_goHome()), this, SLOT(sl_goHome()));

    // System actions
    connect(m_close_button, SIGNAL(clicked()), this, SLOT(sl_quit()));
    connect(m_maximize_or_restore_button, SIGNAL(clicked()), this, SLOT(sl_maximizeOrRestore()));
    connect(m_minimize_button, SIGNAL(clicked()), this, SLOT(showMinimized()));
    connect(m_ui->_MCB_controllBar, SIGNAL(si_moveWindow(QPoint)), this, SLOT(sl_moveWindow(QPoint)));

    // Menu actions
    connect(m_close_act, SIGNAL(triggered()), this, SLOT(close()));
    connect(m_create_assembly_act, SIGNAL(triggered()), this, SLOT(sl_newAssembly()));
    connect(m_save_assembly_act, SIGNAL(triggered()), this, SLOT(sl_saveAssembly()));
    connect(m_update_assembly_properties_act, SIGNAL(triggered()), this, SLOT(sl_saveAssembly()));
    connect(m_app_config_act, SIGNAL(triggered()), this, SLOT(sl_updatePreferences()));
    connect(m_user_manual_act, SIGNAL(triggered()), this, SLOT(sl_showUserManual()));
    connect(m_about_act, SIGNAL(triggered()), this, SLOT(sl_showAboutBox()));
    connect(m_export_assembly_act, SIGNAL(triggered()), this, SLOT(sl_exportAssembly()));
    connect(m_import_assembly_act, SIGNAL(triggered()), this, SLOT(sl_importAssembly()));
    connect(m_export_job_act, SIGNAL(triggered()), this, SLOT(sl_exportJob()));
    connect(m_import_job_act, SIGNAL(triggered()), this, SLOT(sl_importJob()));
    connect(m_day_night_mode_act, SIGNAL(triggered()), this, SLOT(sl_swapDayNightDisplay()));
    connect(m_map_toolbar_act, SIGNAL(triggered()), m_data_viewer, SLOT(sl_showHideToolbar()));
    connect(m_export_map_view_act, SIGNAL(triggered()), this, SLOT(sl_exportMapToImage()));
    connect(m_preprocessing_tool, SIGNAL(triggered()), this, SLOT(sl_launchPreprocessingTool()));
    connect(m_nmea_nav_extrator_tool, SIGNAL(triggered()), this, SLOT(sl_launchNmeaExtractorTool()));
    connect(m_camera_manager_tool, SIGNAL(triggered()), this, SLOT(sl_launchCameraManagerTool()));
    connect(m_camera_calib_tool, SIGNAL(triggered()), this, SLOT(sl_launchCameraCalibTool()));
    
    // Menus contextuels
    connect(m_create_job_act, SIGNAL(triggered()), this, SLOT(sl_newJob()));
    connect(m_save_job_act, SIGNAL(triggered()), this, SLOT(sl_saveJob()));
    connect(m_delete_assembly_act, SIGNAL(triggered()), this, SLOT(sl_deleteAssembly()));
    connect(m_restore_job_act, SIGNAL(triggered()), this, SLOT(sl_restoreJobs()));
    connect(m_clone_assembly_act, SIGNAL(triggered()), this, SLOT(sl_duplicateAssembly()));

    connect(_executeJobAct, SIGNAL(triggered()), this, SLOT(sl_launchJob()));
    connect(m_go_to_results_act, SIGNAL(triggered()), this, SLOT(sl_goToResult()));
    connect(m_upload_data_act, SIGNAL(triggered()), this, SLOT(sl_uploadJobData()));
    connect(m_select_remote_data_act, SIGNAL(triggered()), this, SLOT(sl_selectRemoteJobData()));
    connect(m_execute_remote_job_act, SIGNAL(triggered()), this, SLOT(sl_launchRemoteJob()));
    connect(m_download_job_results_act, SIGNAL(triggered()), this, SLOT(sl_downloadJobResults()));
    connect(m_delete_job_act, SIGNAL(triggered()), this, SLOT(sl_deleteJob()));
    connect(m_archive_job_act, SIGNAL(triggered()), this, SLOT(sl_archiveJob()));
    connect(m_clone_job_act, SIGNAL(triggered()), this, SLOT(sl_duplicateJob()));

    // Tool button actions
    connect(m_stop_button, SIGNAL(clicked()), this, SLOT(sl_stopJob()));
    connect(m_ui->_PB_parameterFold, SIGNAL(clicked(bool)), this, SLOT(sl_foldUnfoldParameters()));
}

void MainGui::initEngine()
{
    m_engine.init();

    connect(&m_engine, SIGNAL(si_jobProcessed(QString, bool)), this, SLOT(sl_jobProcessed(QString, bool)));
    connect(&m_engine, SIGNAL(si_jobShowImageOnMainView(QString,Image *)), this, SLOT(sl_jobShowImageOnMainView(QString,Image *)));
    connect(&m_engine, SIGNAL(si_userInformation(QString)), this, SLOT(sl_userInformation(QString)));
    connect(&m_engine, SIGNAL(si_processCompletion(quint8)), this, SLOT(sl_processCompletion(quint8)));
}

void MainGui::initAssemblyCreationScene()
{
    m_assembly_editor->getScene()->setEngine(&m_engine);
    m_assembly_editor->getScene()->setElementWidgetProvider(this);
    m_assembly_editor->getScene()->setMessageTarget(this);

    connect(m_assembly_editor->getScene(), SIGNAL(si_assemblyModified()), this, SLOT(sl_modifiedAssembly()));
    connect(m_assembly_editor->getScene(), SIGNAL(si_assemblyComplete(bool)), this, SLOT(sl_assemblyComplete(bool)));
}

void MainGui::initWelcomeDialog()
{   
    m_welcome_dialog = new WelcomeDialog(this, m_icon_factory, m_preferences->programmingModeEnabled());
    m_welcome_dialog->setObjectName("_D_welcomeDialog");
    m_welcome_dialog->setWindowFlags(Qt::Dialog | Qt::FramelessWindowHint);
    //_welcomeDialog->show();
}

void MainGui::initMapFeatures()
{
    m_data_viewer->setIconFactory(m_icon_factory);
    m_data_viewer->initCanvas();
    m_data_viewer->initMapToolBar();
}

void MainGui::dpiScaleWidgets()
{
    GraphicalCharter &graph_charter = GraphicalCharter::instance();
    int dpi_cb_height = graph_charter.dpiScaled(CONTROLLBAR_HEIGHT);

    // Adjust main window size
    this->setMinimumWidth(graph_charter.dpiScaled(MAIN_WINDOW_MIN_WIDTH));
    this->setMinimumHeight(graph_charter.dpiScaled(MAIN_WINDOW_MIN_HEIGHT));

    // Main control bar scaling
    m_ui->_MCB_controllBar->setFixedHeight(dpi_cb_height);

    m_home_widget->setFixedWidth(graph_charter.dpiScaled(CB_HOME_BUTTON_WIDTH));
    m_home_widget->setFixedHeight(dpi_cb_height);
    m_home_button->setFixedWidth(graph_charter.dpiScaled(CB_HOME_BUTTON_WIDTH));
    m_home_button->setFixedHeight(dpi_cb_height);
    m_home_button->setIconSize(QSize(graph_charter.dpiScaled(CB_HOME_BUTTON_ICON),graph_charter.dpiScaled(CB_HOME_BUTTON_ICON)));
    m_visu_mode_button->setIconSize(QSize(graph_charter.dpiScaled(CB_VISU_SWAP_ICON),graph_charter.dpiScaled(CB_HOME_BUTTON_ICON)));

    findChild<MatisseVersionWidget*>(QString("matisseVersionWidget"))->setFixedHeight(dpi_cb_height);
    findChild<MatisseVersionWidget*>(QString("matisseVersionWidget"))->setFixedWidth(graph_charter.dpiScaled(CB_VERSION_WIDTH));
    m_matisse_version_label->setFixedHeight(dpi_cb_height);

    findChild<VisuModeWidget*>(QString("visuModeWidget"))->setFixedHeight(dpi_cb_height);
    findChild<VisuModeWidget*>(QString("visuModeWidget"))->setFixedWidth(graph_charter.dpiScaled(CB_VISU_INFO_WIDTH));
    m_active_view_or_mode_label->setFixedHeight(graph_charter.dpiScaled(CONTROLLBAR_HEIGHT/2));
    m_current_date_time_label->setFixedHeight(1+graph_charter.dpiScaled(CONTROLLBAR_HEIGHT/2)); // +1 is for the rounding

    findChild<QWidget*>(QString("mainMenuWidget"))->setFixedHeight(dpi_cb_height);
    findChild<QMenuBar*>(QString("_MBA_mainMenuBar"))->setFixedHeight(graph_charter.dpiScaled(CONTROLLBAR_HEIGHT/2));

    OngoingProcessWidget* proc_wid = findChild<OngoingProcessWidget*>(QString("ongoingProcessWidget"));
    proc_wid->setFixedHeight(dpi_cb_height);
    proc_wid->setFixedWidth(graph_charter.dpiScaled(CB_ON_PROCESS_WIDTH));
    proc_wid->dpiScale();

    // Right panel
    m_ui->_PB_parameterFold->setFixedWidth(dpi_cb_height/3);
    m_parameters_dock->setFixedWidth(graph_charter.dpiScaled(CB_ON_PROCESS_WIDTH)-m_ui->_PB_parameterFold->width());

    // Left panel
    m_ui->_SPLIT_leftMenu->setFixedWidth(graph_charter.dpiScaled(CB_HOME_BUTTON_WIDTH+CB_VERSION_WIDTH));

    this->setGeometry(
                QStyle::alignedRect(
                    Qt::LeftToRight,
                    Qt::AlignCenter,
                    this->size(),
                    qApp->desktop()->availableGeometry()
                    )
                );

}

void MainGui::init()
{
    initLanguages();
    initIconFactory();
    initStatusBar();
    lookupChildWidgets();
    initStylesheetSelection();
    resetOngoingProcessIndicators(); // hide current process indicators
    initMainMenu();
    initContextMenus();
    initProcessWheelSignalling();
    initUserActions();
    initEngine();
    initVersionDisplay();
    initParametersWidget();
    initPreferences();
    initAssemblyHelper();
    initJobHelper();
    initImportExportHelper();
    initRemoteJobHelper();
    initAssemblyCreationScene();
    initMapFeatures();
    initProcessorWidgets(); // loading processors
    sl_swapMapOrCreationView(); // swap to map view

    /* notify widgets for initial color palette */
    emit si_updateColorPalette(m_current_color_set);

    initDateTimeDisplay(); // start current date/time timer
    initWelcomeDialog();
    dpiScaleWidgets();
}

void MainGui::initAssemblyHelper()
{
    m_assembly_helper->setEngine(&m_engine);
}

void MainGui::initJobHelper()
{
    m_job_helper->setEngine(&m_engine);
    m_job_helper->setPreferences(m_preferences);
    m_job_helper->setIconFactory(m_icon_factory);
}

void MainGui::initImportExportHelper()
{
    m_import_export_helper->init();
}

void MainGui::initRemoteJobHelper()
{
  m_remote_job_helper->setJobLauncher(this);
  m_remote_job_helper->setParametersManager(m_engine.parametersManager());
  m_remote_job_helper->setPreferences(m_preferences);
  m_remote_job_helper->setServerSettings(SystemDataManager::instance()->remoteServerSettings());
  m_remote_job_helper->init();
  connect(m_remote_job_helper, SIGNAL(si_jobResultsReceived(QString)),
          SLOT(sl_onRemoteJobResultsReceived(QString)));
}

void MainGui::initIconFactory()
{
    m_current_color_set = FileUtils::readPropertiesFile("lnf/MatisseColorsDay.properties");
    m_icon_factory = new MatisseIconFactory(m_current_color_set, "grey", "orange", "orange2");
    connect(this, SIGNAL(si_updateColorPalette(QMap<QString,QString>)), m_icon_factory, SLOT(sl_updateColorPalette(QMap<QString,QString>)));
    connect(this, SIGNAL(si_updateExecutionStatusColor(QString)), m_icon_factory, SLOT(sl_updateExecutionStatusColor(QString)));
    connect(this, SIGNAL(si_updateAppModeColors(QString,QString)), m_icon_factory, SLOT(sl_updateAppModeColors(QString,QString)));
}

void MainGui::initStylesheetSelection()
{
    setStyleSheet("");

    m_stylesheet_by_app_mode.insert(PROGRAMMING, "lnf/MatisseModeProg.css");
    m_stylesheet_by_app_mode.insert(POST_PROCESSING, "lnf/MatisseModeDt.css");
    m_stylesheet_by_app_mode.insert(APP_CONFIG, "lnf/MatisseModeProg.css");

    m_wheel_colors_by_mode.insert(PROGRAMMING, QString("%1-%2-%3").arg("<%color.grey%>").arg("<%color.orange%>").arg("<%color.orange2%>"));
    m_wheel_colors_by_mode.insert(POST_PROCESSING, QString("%1-%2-%3").arg("<%color.grey%>").arg("<%color.mauve%>").arg("<%color.mauve2%>"));
    m_wheel_colors_by_mode.insert(APP_CONFIG, QString("%1-%2-%3").arg("<%color.grey%>").arg("<%color.orange%>").arg("<%color.orange2%>"));

    m_colors_by_mode1.insert(PROGRAMMING, QString("orange"));
    m_colors_by_mode1.insert(POST_PROCESSING, QString("mauve"));
    m_colors_by_mode1.insert(APP_CONFIG, QString("grey"));

    m_colors_by_mode2.insert(PROGRAMMING, QString("orange2"));
    m_colors_by_mode2.insert(POST_PROCESSING, QString("mauve2"));
    m_colors_by_mode2.insert(APP_CONFIG, QString("grey2"));

    m_colors_by_level.insert(IDLE, QString("grey"));
    m_colors_by_level.insert(OK, QString("level.green"));
    m_colors_by_level.insert(WARNING, QString("level.orange"));
    m_colors_by_level.insert(ERR, QString("level.red"));

    IconizedWidgetWrapper *close_button_wrapper = new IconizedButtonWrapper(m_close_button);
    m_icon_factory->attachIcon(close_button_wrapper, "lnf/icons/fermer.svg", false, false);

    IconizedWidgetWrapper *minimize_button_wrapper = new IconizedButtonWrapper(m_minimize_button);
    m_icon_factory->attachIcon(minimize_button_wrapper, "lnf/icons/reduire.svg", false, false);

    m_max_or_restore_button_wrapper = new IconizedButtonWrapper(m_maximize_or_restore_button);
    m_icon_factory->attachIcon(m_max_or_restore_button_wrapper, "lnf/icons/agrandir.svg", false, false);

    IconizedWidgetWrapper *stop_button_wrapper = new IconizedButtonWrapper(m_stop_button);
    m_icon_factory->attachIcon(stop_button_wrapper, "lnf/icons/Main.svg", false, true);

    IconizedWidgetWrapper *home_button_wrapper = new IconizedButtonWrapper(m_home_button);
    m_icon_factory->attachIcon(home_button_wrapper, "lnf/icons/Maison.svg", false, false);

    m_visu_mode_button_wrapper = new IconizedButtonWrapper(m_visu_mode_button);
    m_icon_factory->attachIcon(m_visu_mode_button_wrapper, "lnf/icons/Cartographie.svg", false, true);

    IconizedWidgetWrapper *reset_messages_button_wrapper = new IconizedButtonWrapper(m_reset_messages_button);
    m_icon_factory->attachIcon(reset_messages_button_wrapper, "lnf/icons/trash.svg", false, false);

    IconizedWidgetWrapper *messages_picto_wrapper = new IconizedLabelWrapper(m_messages_picto);
    m_icon_factory->attachIcon(messages_picto_wrapper, "lnf/icons/Message.svg", true, true);

    /* Day / night signalling for local widgets */
    connect(this, SIGNAL(si_updateColorPalette(QMap<QString,QString>)), m_ui->_PB_parameterFold, SLOT(sl_updateColorPalette(QMap<QString,QString>)));
    connect(this, SIGNAL(si_updateColorPalette(QMap<QString,QString>)), m_data_viewer, SLOT(sl_updateColorPalette(QMap<QString,QString>)));
}

void MainGui::initMainMenu()
{
    /* Identifying container widget */
    QWidget* menu_container = findChild<QWidget*>(QString("mainMenuWidget"));

    /* FILE MENU */
    m_file_menu = new MatisseMenu(menu_container);

    m_export_map_view_act = new QAction(this);
    m_close_act = new QAction(this);

    m_file_menu->addAction(m_export_map_view_act);
    m_file_menu->addSeparator();
    m_file_menu->addAction(m_close_act);

    /* DISPLAY MENU */
    m_display_menu = new MatisseMenu(menu_container);

    m_day_night_mode_act = new QAction(this);
    m_day_night_mode_act->setCheckable(true);
    m_day_night_mode_act->setChecked(false);

    m_map_toolbar_act = new QAction(this);
    m_map_toolbar_act->setCheckable(true);
    m_map_toolbar_act->setChecked(false);

    m_display_menu->addAction(m_day_night_mode_act);
    m_display_menu->addSeparator();
    m_display_menu->addAction(m_map_toolbar_act);

    /* PROCESSING MENU */
    m_process_menu = new MatisseMenu(menu_container);

    m_create_assembly_act = new QAction(this);
    m_save_assembly_act = new QAction(this);
    m_import_assembly_act = new QAction(this);
    m_export_assembly_act = new QAction(this);

    m_process_menu->addAction(m_create_assembly_act);
    m_process_menu->addAction(m_save_assembly_act);
    m_process_menu->addSeparator();
    m_process_menu->addAction(m_import_assembly_act);
    m_process_menu->addAction(m_export_assembly_act);

    /* TOOLS MENU */
    m_tool_menu = new MatisseMenu(menu_container);

    m_app_config_act = new QAction(this);
    m_preprocessing_tool = new QAction(this);
    m_nmea_nav_extrator_tool = new QAction(this);
    m_camera_manager_tool = new QAction(this);
    m_camera_calib_tool = new QAction(this);

    m_tool_menu->addAction(m_app_config_act);
    m_tool_menu->addSeparator();
    m_tool_menu->addAction(m_preprocessing_tool);
    m_tool_menu->addAction(m_nmea_nav_extrator_tool);
    m_tool_menu->addSeparator();
    m_tool_menu->addAction(m_camera_manager_tool);
    m_tool_menu->addAction(m_camera_calib_tool);
    m_tool_menu->addSeparator();


    /* HELP MENU */
    m_help_menu = new MatisseMenu(menu_container);

    m_user_manual_act = new QAction(this);
    m_about_act = new QAction(this);

    m_help_menu->addAction(m_user_manual_act);
    m_help_menu->addAction(m_about_act);

    QMenuBar* main_menu_bar = findChild<QMenuBar*>(QString("_MBA_mainMenuBar"));
    main_menu_bar->addMenu(m_file_menu);
    main_menu_bar->addMenu(m_display_menu);
    main_menu_bar->addMenu(m_process_menu);
    main_menu_bar->addMenu(m_tool_menu);
    main_menu_bar->addMenu(m_help_menu);
}

void MainGui::initContextMenus()
{
    /* Actions pour menu contextuel Traitement */
    m_create_job_act = new QAction(this);
    m_import_job_act = new QAction(this);
    m_clone_assembly_act = new QAction(this);
    m_delete_assembly_act = new QAction(this);
    m_restore_job_act = new QAction(this);
    m_update_assembly_properties_act = new QAction(this);

    /* Actions pour menu contextuel Tâche */
    _executeJobAct = new QAction(this);
    m_execute_remote_job_act = new QAction(this);
    m_upload_data_act = new QAction(this);
    m_select_remote_data_act = new QAction(this);
    m_download_job_results_act = new QAction(this);
    m_save_job_act = new QAction(this);
    m_clone_job_act = new QAction(this);
    m_export_job_act = new QAction(this);
    m_delete_job_act = new QAction(this);
    m_archive_job_act = new QAction(this);
    m_go_to_results_act = new QAction(this);

    m_ui->_TRW_assemblies->setContextMenuPolicy(Qt::CustomContextMenu);
}

void MainGui::initVersionDisplay()
{
    m_app_version = SystemDataManager::instance()->getVersion();
    QStringList version_items = m_app_version.split(".");

    if (version_items.size() < 3) {
        qCritical() << QString("Version '%1' defined in XML settings does not conform to the 'X.Y.Z' pattern, default version will be displayed").arg(m_app_version);
        return;
    }

    QString product_serie = version_items.at(0);
    QString major = version_items.at(1);
    QString minor = version_items.at(2);

    QString template_label = "MATISSE 3D\nV%1.%2.%3";
    QString full_version_label = template_label.arg(product_serie).arg(major).arg(minor);

    m_matisse_version_label->setText(full_version_label);
}

bool MainGui::loadResultToCartoView(QString _result_file_p, bool _remove_previous_scenes)
{
    QFileInfo info_result(_result_file_p);

    if (!info_result.exists()) {
        qCritical() << "Result image file not found " << info_result.absoluteFilePath();
        return false;
    }

    if (m_data_viewer->supportedRasterFormat().contains(info_result.suffix())){
        //qDebug() << "Loading raster layer " << resultFile_p;
        m_data_viewer->loadRasterFile(info_result.absoluteFilePath());

    }else if (m_data_viewer->supportedVectorFormat().contains(info_result.suffix())){
        qDebug() << "Vector layer not supported anymore" << _result_file_p;
        //_userFormWidget->loadShapefile(infoResult.absoluteFilePath());

    }else if (m_data_viewer->supported3DFileFormat().contains(info_result.suffix())){
        m_data_viewer->invokeThreaded3DFileLoader(info_result.absoluteFilePath(), _remove_previous_scenes);

    }else if (m_data_viewer->supportedImageFormat().contains(info_result.suffix())){
        //qDebug() << "Loading image file " << resultFile_p;
        m_data_viewer->loadImageFile(info_result.absoluteFilePath());

    }else{
        //qDebug() << "Output file format not supported";
        return false;
    }

    return true;

}


void MainGui::loadAssembliesAndJobsLists(bool _do_expand)
{
    /* Clearing tree and lists */
    m_ui->_TRW_assemblies->clear();
    m_assemblies_items.clear();

    /* free all tree item wrappers attached to icons */
    m_icon_factory->clearObsoleteIcons();

    // free properties key value lists memory allocation
    QList<KeyValueList*> props = m_assemblies_properties.values();
    foreach (KeyValueList* prop, props) {
        delete prop;
    }
    m_assemblies_properties.clear();

    /* Load elements */
    ProcessDataManager* process_data_manager = ProcessDataManager::instance();
    process_data_manager->loadAssembliesAndJobs();
    process_data_manager->loadArchivedJobs(m_job_helper->archivePath());

    /* Building assembly tree */
    QStringList valid_assemblies = process_data_manager->getAssembliesList();

    foreach(QString assembly_name, valid_assemblies) {
        AssemblyDefinition *assembly = process_data_manager->getAssembly(assembly_name);
        addAssemblyInTree(assembly);
    }

    /* Add jobs to assemblies tree */
    QStringList jobs_names = process_data_manager->getJobsNames();
    foreach(QString job_name, jobs_names) {
        JobDefinition * job_def = process_data_manager->getJob(job_name);

        if (job_def) {
            addJobInTree(job_def);
        }
    }

    m_ui->_TRW_assemblies->sortItems(0, Qt::AscendingOrder);
    if (_do_expand) {
        m_ui->_TRW_assemblies->expandAll();
    }

    /* If new assembly is pending, add it to tree */
    /* (This sould be prevented by user action checks */
    if (m_new_assembly) {
        addAssemblyInTree(m_new_assembly);
    }
}

void MainGui::loadStyleSheet(eApplicationMode _mode)
{
    emit si_updateAppModeColors(m_colors_by_mode1.value(_mode), m_colors_by_mode2.value(_mode));

    // Selecting mode-specific stylesheet
    QString style_sheet_for_mode = m_stylesheet_by_app_mode.value(_mode);

    // Loading stylesheets...

    QFile generic_style_sheet("lnf/Matisse.css");
    QFile mode_specific_style_sheet(style_sheet_for_mode);

    if (!generic_style_sheet.exists() || !mode_specific_style_sheet.exists()) {
        qWarning() << "Stylesheet files not found";
    } else {
        if (!generic_style_sheet.open(QIODevice::ReadOnly)) {
            qWarning() << "Generic stylesheet file could not be opened\n" << generic_style_sheet.error();
        } else {
            QByteArray global_styles_bytes = generic_style_sheet.readAll();
            generic_style_sheet.close();

            if(!mode_specific_style_sheet.open(QIODevice::ReadOnly)) {
                qWarning() << "Mode specific stylesheet file could not be opened\n" << mode_specific_style_sheet.error();
            } else {
                // appending specific styles to the global stylesheet
                global_styles_bytes.append(mode_specific_style_sheet.readAll());
                mode_specific_style_sheet.close();
            }

            // Substituting stylesheet variables
            QString global_styles_with_variables(global_styles_bytes);
            QString global_styles = StringUtils::substitutePlaceHolders(global_styles_with_variables, m_current_color_set);

            // Applying stylesheet
            qApp->setStyleSheet(global_styles);
        }
    }

    // EXTRA CUSTOMISATIONS

    /* Live process wheel */
    QString wheel_colors_template = m_wheel_colors_by_mode.value(_mode);
    // resolving day/night colors
    QString wheel_colors = StringUtils::substitutePlaceHolders(wheel_colors_template, m_current_color_set);
    emit si_updateWheelColors(wheel_colors);

    /* activation des actions selon mode applicatif */
    bool has_expert_features = (_mode == PROGRAMMING);

    m_visu_mode_button->setVisible(has_expert_features);
    m_create_assembly_act->setVisible(has_expert_features);
    m_save_assembly_act->setVisible(has_expert_features);

    // affichage du mode pour TD (sinon affichage de la vue)
    if(_mode == POST_PROCESSING){
        m_active_view_or_mode_label->setText(tr("Mode : Post-processing"));
    }

    // always init application mode with map view
    if (!m_is_map_view) {
        sl_swapMapOrCreationView();
    } else {
        if (m_active_application_mode == PROGRAMMING) {
            m_active_view_or_mode_label->setText(tr("View : Cartography"));
        }

        m_context.setLastActionPerformed(CHANGE_APP_MODE);
        enableActions();
    }

}


void MainGui::loadDefaultStyleSheet()
{
    loadStyleSheet(POST_PROCESSING);
}


bool MainGui::promptAssemblyNotSaved() {

    bool confirm_action = true;

    if (m_new_assembly) {

        QString new_assembly_name = getActualNewAssemblyName();

        int user_response = QMessageBox::question(this, tr("New assembly..."),
                                                 tr("Assembly '%1' not yet saved.\nContinue anyway ?\n(Will delete the current assembly)")
                                                 .arg(new_assembly_name), QMessageBox::Yes | QMessageBox::No, QMessageBox::No);

        if (user_response==QMessageBox::Yes) {
            deleteAssemblyAndReload(false);
        } else {
            confirm_action = false;
        }

    } else if (m_is_assembly_modified) {
        if (!m_current_assembly) {
            qCritical() << "Assembly was modified (parameters or processing chain) but the current assembly is unknown, modifications if any will be lost";

            /* reset modification flag */
            m_is_assembly_modified = false;
            return true;
        }

        QString current_assembly_name = m_current_assembly->name();

        int user_response = QMessageBox::question(this, tr("Assembly modified..."), tr("Assembly '%1' changed.\nContinue anyway ?").arg(current_assembly_name), QMessageBox::Yes | QMessageBox::No, QMessageBox::No);

        if (user_response == QMessageBox::No) {
            confirm_action = false;
        }

    }

    return confirm_action;
}


void MainGui::checkAndSelectAssembly(QString _selected_assembly_name)
{
    if (m_new_assembly) {

        QString new_assembly_name = getActualNewAssemblyName();
        selectAssembly(new_assembly_name, false);

        bool confirm_action = promptAssemblyNotSaved();

        if (confirm_action) {
            /* restore selection */
            selectAssembly(_selected_assembly_name);
        }

    } else if (m_is_assembly_modified) {
        if (!m_current_assembly) {
            qCritical() << "Assembly parameters were modified but the current assembly is unknown, modifications if any will be lost";

            /* reset modification flag */
            m_is_assembly_modified = false;
            return;
        }

        QString current_assembly_name = m_current_assembly->name();

        /* temporarily cancel selection */
        selectAssembly(current_assembly_name, false);

        bool confirm_action = promptAssemblyNotSaved();

        if (confirm_action) {
            /* reset modification flag */
            m_is_assembly_modified = false;

            /* mark job as saved */
            handleAssemblyModified();

            /* restore selection */
            selectAssembly(_selected_assembly_name);
        }

    } else {
        selectAssembly(_selected_assembly_name);
    }

}


void MainGui::promptJobNotSaved()
{
    if (m_job_parameter_modified) {
        if (!m_current_job) {
            qCritical() << "Job parameters were modified but the current job is unknown, modifications if any will be lost";

            /* reset modification flag */
            m_job_parameter_modified = false;

        } else {
            QString current_job_name = m_current_job->name();

            int user_response = QMessageBox::question(this, tr("Parameters modification..."), tr("Parameters from task '%1' were modified.\nSave mods before going on ?").arg(current_job_name), QMessageBox::Yes | QMessageBox::No, QMessageBox::Yes);

            if (user_response == QMessageBox::Yes) {
                // save job parameters
                m_engine.parametersManager()->saveParametersValues(current_job_name, false);
            }

            /* reset modification flag */
            m_job_parameter_modified = false;

            /* mark job as saved */
            handleJobModified();
        }
    }

}


void MainGui::checkAndSelectJob(QTreeWidgetItem* _selected_item)
{
    if (m_job_parameter_modified) {
        if (!m_current_job) {
            qCritical() << "Job parameters were modified but the current job is unknown, modifications if any will be lost";

            /* reset modification flag */
            m_job_parameter_modified = false;

        } else {
            QString current_job_name = m_current_job->name();

            /* temporarily cancel selection */
            selectJob(current_job_name, false);

            /* handle user choice to save job parameters or not */
            promptJobNotSaved();

            /* restore selection */
            m_ui->_TRW_assemblies->setCurrentItem(_selected_item);
        }

    }
}

void MainGui::resetOngoingProcessIndicators()
{
    // Reset and hide ongoing process indicator on selection of another assembly / job
    m_ongoing_process_info_label->setText("");
    m_ongoing_process_completion->setValue(0);
    m_ongoing_process_info_label->hide();
    m_ongoing_process_completion->hide();
    emit si_processStopped(); // to reset wheel if previous job was frozen
}

void MainGui::sl_selectAssemblyOrJob(QTreeWidgetItem * _selected_item, int _column)
{
    Q_UNUSED(_column)

    if (!_selected_item)
        return;

    if (m_is_map_view) {
        resetOngoingProcessIndicators();
    }

    if (!_selected_item->parent()) { // An assembly was selected

        QString selected_assembly_name = getActualAssemblyOrJobName(_selected_item);

        /* check if same assembly is selected */
        if (m_new_assembly) {
            QString new_assembly_name = getActualNewAssemblyName();

            if (selected_assembly_name == new_assembly_name) {
                //qDebug() << "Same assembly selected";
                return;
            }
        } else if (m_current_assembly) {
            if (selected_assembly_name == m_current_assembly->name()) {
                //qDebug() << "Same assembly selected";
                return;
            }
        }

        checkAndSelectAssembly(selected_assembly_name);
        doFoldUnfoldParameters(!m_is_map_view);

        if (m_is_map_view) {
            // disable parameters editing
            m_engine.parametersManager()->toggleReadOnlyMode(true);

        } else { // Creation view

            m_assembly_editor->getGraphicsView()->setEnabled(true); // activate graphical view
            m_save_assembly_act->setEnabled(true);
        }

        m_context.setLastActionPerformed(SELECT_ASSEMBLY);
        enableActions();

    } else { // A job was selected


        QString selected_job_name = getActualAssemblyOrJobName(_selected_item);

        /* check if same job is selected */
        if (m_current_job) {
            if (selected_job_name == m_current_job->name()) {
                //qDebug() << "Same job selected";
                return;
            }
        }

        checkAndSelectJob(_selected_item);
        displayJob(selected_job_name);
        applyNewApplicationContext();
        m_engine.parametersManager()->toggleReadOnlyMode(false); // enable parameters editing
        doFoldUnfoldParameters(true);

        m_export_assembly_act->setEnabled(false);

        m_context.setLastActionPerformed(SELECT_JOB);
        enableActions();
    }
}

void MainGui::sl_updateTimeDisplay()
{
    QDateTime current = QDateTime::currentDateTime();
    m_current_date_time_label->setText(current.toString("dd/MM/yyyy hh:mm"));
}

void MainGui::sl_updatePreferences()
{
    /* hide parameters view to display the dialog on top of the dark background */
    bool previous_folding_state = m_ui->_PB_parameterFold->getIsUnfolded();
    doFoldUnfoldParameters(false);

    PreferencesDialog dialog(this, m_icon_factory, m_preferences, false);
    dialog.setFixedHeight(GraphicalCharter::instance().dpiScaled(PD_HEIGHT));
    dialog.setFixedWidth(GraphicalCharter::instance().dpiScaled(PD_WIDTH));
    dialog.setWindowFlags(Qt::Dialog | Qt::FramelessWindowHint);

    /* new preferences are saved if modified */
    if (dialog.exec() == QDialog::Accepted) {
        SystemDataManager::instance()->writeMatissePreferences(PREFERENCES_FILEPATH, *m_preferences);
        ProcessDataManager::instance()->loadArchivedJobs(m_job_helper->archivePath());
        updateLanguage(m_preferences->language());
        updatePreferredDatasetParameters();

        /* recheck preferences for remote job execution */
        m_remote_job_helper->reinit();
    }

    /* restore parameters view to initial  state */
    doFoldUnfoldParameters(previous_folding_state);
}

void MainGui::sl_foldUnfoldParameters()
{    
    bool is_unfold_action = m_ui->_PB_parameterFold->getIsUnfolded();
    doFoldUnfoldParameters(is_unfold_action, true);
}

void MainGui::sl_showUserManual()
{
    QString user_manual_file_name = "help/MatisseHelp_" + m_current_language + ".pdf";

    QFileInfo user_manual_file(user_manual_file_name);

    if (!user_manual_file.exists()) {
        QMessageBox::warning(this, tr("User manual"), tr("User manual file '%1' does not exist")
                             .arg(user_manual_file.absoluteFilePath()));
        return;
    }

    QUrl url = QUrl::fromLocalFile(user_manual_file.absoluteFilePath());
    QDesktopServices::openUrl(url);
}

void MainGui::sl_showAboutBox()
{
    KeyValueList meta;
    meta.append("version", m_app_version);

    AboutDialog about(this, meta);
    about.setWindowFlags(Qt::Dialog | Qt::FramelessWindowHint);
    if (about.exec() != QDialog::Accepted) {
        return;
    }
}


void MainGui::updateJobStatus(
    QString _job_name, QTreeWidgetItem *_item, eJobStatusMessageLevel _indicator,
    QString _message) 
{
  if (_item->type() == MatisseTreeItem::Type) {
    MatisseTreeItem *matisse_job_item = static_cast<MatisseTreeItem *>(_item);
    IconizedWidgetWrapper *item_wrapper =
        new IconizedTreeItemWrapper(matisse_job_item, 0);
    m_icon_factory->attachIcon(item_wrapper, "lnf/icons/led.svg", false, false,
                             m_colors_by_level.value(_indicator));
  } else {
    qCritical() << QString(
                       "Item for job '%1' is not of type MatisseTreeItem, "
                       "cannot display icon")
                       .arg(_job_name);
  }

  QString msg = _message.arg(_job_name);
  showStatusMessage(msg, OK);
}

void MainGui::sl_exportAssembly()
{
    executeExportWorkflow(false);
}

void MainGui::sl_importAssembly()
{
    executeImportWorkflow();
}

void MainGui::sl_exportJob()
{
    executeExportWorkflow(true);
}

void MainGui::sl_importJob()
{
    executeImportWorkflow(true);
}

void MainGui::sl_goToResult()
{
  /* Guard unconsistent cases (this slot is assumed by selecting a previously executed job) */
  if (!m_current_job) {
    qCritical() << "No job was selected, cannot open reconstruction folder";
    return;  
  }

  if (!m_current_job->executionDefinition()->executed()) {
    qCritical() << "Job was selected, cannot open reconstruction folder";
    return;
  }

  QStringList result_files = m_current_job->executionDefinition()->resultFileNames();
  if (result_files.isEmpty()) {
    qCritical() << "Job has no result files, cannot open reconstruction folder";
    return;
  }

  /* Reconstruction folder is the parent path of all result images (wether executed locally or remotely) */
  QString result_file = result_files.first();
  int file_name_pos = result_file.lastIndexOf('/') + 1;
  QString result_path = result_file.left(file_name_pos);

  QDir result_dir(result_path);
  if (!result_dir.exists()) {
      QMessageBox::critical(this, tr("Path invalid"), tr("Result path '%1' does not exist.").arg(result_path));
      return;
  }

  QUrl url = QUrl::fromLocalFile(result_dir.absolutePath());
  QDesktopServices::openUrl(url);
}

void MainGui::sl_archiveJob()
{
    if (!m_current_job) {
        // technically inconsistent
        qCritical() << QString("Current job not identified, could not archive");
        return;
    }

    if (m_job_helper->archiveJob(m_current_job)) {
        sl_assembliesReload();
    }
}

void MainGui::sl_restoreJobs()
{
    if (!m_current_assembly) {
        qCritical() << "Could not identify selected assembly, impossible to restore jobs";
        return;
    }

    if (m_job_helper->restoreJobs(m_current_assembly)) {
        sl_assembliesReload();
    }
}

void MainGui::sl_duplicateJob()
{
    if (!m_current_job) {
        qCritical() << "The selected job cannot be identified, impossible to duplicate";
        return;
    }

    if (m_job_helper->duplicateJob(m_current_job)) {
        sl_assembliesReload();
    }
}

void MainGui::sl_duplicateAssembly()
{
    if (!m_current_assembly) {
        qCritical() << "The selected assembly cannot be identified, impossible to duplicate";
        return;
    }

    if (m_assembly_helper->duplicateAssembly(m_current_assembly)) {
        sl_assembliesReload();
    }
}

void MainGui::sl_swapDayNightDisplay()
{
    m_is_night_display_mode = !m_is_night_display_mode;

    QString colors_file_path;
    if (m_is_night_display_mode) {
        colors_file_path = "lnf/MatisseColorsNight.properties";
    } else {
        colors_file_path = "lnf/MatisseColorsDay.properties";
    }

    /* Reload stylesheet with new color set */
    m_current_color_set = FileUtils::readPropertiesFile(colors_file_path);
    loadStyleSheet(m_active_application_mode);

    /* notify other widgets */
    emit si_updateColorPalette(m_current_color_set);
}

void MainGui::sl_exportMapToImage()
{
    QString export_path = m_import_export_helper->exportPath();
    QString image_file_path = QFileDialog::getSaveFileName(this, tr("Export current view to image..."), export_path, tr("Image file (*.png)"));

    if (image_file_path.isEmpty()) {
        /* cancel */
        return;
    }

    m_data_viewer->exportMapViewToImage(image_file_path);

    QMessageBox::information(
                this,
                tr("Export view to image"),
                tr("View has been exported in file %1").arg(image_file_path));
}


void MainGui::sl_launchPreprocessingTool()
{
    QMap<QString, QString> external_tools = SystemDataManager::instance()->getExternalTools();
    if (!external_tools.contains("preprocessingTool")) {
        qCritical() << "preprocessingTool tool not defined in settings";
        QMessageBox::critical(this, tr("Incomplete system configuration"), tr("Preprocessing tool not defined in system configuration"));
        return;
    }

    QString tool_path = external_tools.value("preprocessingTool");
    QFileInfo tool_path_file(tool_path);

    if (!tool_path_file.exists()) {
        qCritical() << QString("Could not find preprocessing tool exe file '%1'").arg(tool_path);
        QMessageBox::critical(this, tr("Tool not found"), tr("Preprocessing tool not found in file '%1'").arg(tool_path));
        return;
    }

    QUrl url = QUrl::fromLocalFile(tool_path_file.absoluteFilePath());
    QDesktopServices::openUrl(url);
}

void MainGui::sl_launchNmeaExtractorTool()
{
    QMap<QString, QString> external_tools = SystemDataManager::instance()->getExternalTools();
    if (!external_tools.contains("nmeaNavExtractor")) {
        qCritical() << "nmeaNavExtractor tool not defined in settings";
        QMessageBox::critical(this, tr("Incomplete system configuration"), tr("Preprocessing tool not defined in system configuration"));
        return;
    }

    QString tool_path = external_tools.value("nmeaNavExtractor");
    QFileInfo tool_path_file(tool_path);

    if (!tool_path_file.exists()) {
        qCritical() << QString("Could not find NmeaNavExtractor tool exe file '%1'").arg(tool_path);
        QMessageBox::critical(this, tr("Tool not found"), tr("NmeaNavExtractor tool not found in file '%1'").arg(tool_path));
        return;
    }

    QUrl url = QUrl::fromLocalFile(tool_path_file.absoluteFilePath());
    QDesktopServices::openUrl(url);
}

void MainGui::sl_launchCameraManagerTool()
{
    m_camera_manager_tool_dialog.show();
}

void MainGui::sl_launchCameraCalibTool()
{
    m_camera_calib_tool_dialog.show();
}


void MainGui::executeExportWorkflow(bool _is_job_export_action, bool _is_for_remote_execution) {
    if (_is_for_remote_execution && ! _is_job_export_action) {
        qCritical() << "Unconsistent call cannot be executed : assembly export for remote execution";
        return;
    }

    /* check that an assembly/job is selected */
    bool selected = (_is_job_export_action) ? (m_current_job != NULL) : (m_current_assembly != NULL);

    if (!selected) {
        QString export_failed_title = tr("Export failure");
        QString no_entity_selected_message = (_is_job_export_action) ? tr("No task selected.") : tr("No processing chain were selected.");

        qCritical() << "No assembly/job selected, impossible to export";
        QMessageBox::critical(this, export_failed_title, no_entity_selected_message);
        return;
    }

    /* check unsaved assembly / job */
    bool confirm_action = true;

    if (_is_job_export_action) {
        if (m_is_map_view && m_job_parameter_modified) {
            promptJobNotSaved();
        }
    } else {
        if (!m_is_map_view) {
            if (m_is_assembly_modified || m_new_assembly) {
                confirm_action = promptAssemblyNotSaved();
            }
        }
    }

    if (!confirm_action) {
        return;
    }

    QString entity_name = (_is_job_export_action) ? m_current_job->name() : m_current_assembly->name();
    if (_is_for_remote_execution) {
        m_import_export_helper->executeExportWorkflow(_is_job_export_action, entity_name, true, JOB_REMOTE_PREFIX, m_remote_job_helper->remoteOutputPath());
        m_current_remote_execution_bundle = m_import_export_helper->customExportFilename();
    } else {
        m_import_export_helper->executeExportWorkflow(_is_job_export_action, entity_name, false);
    }
}


void MainGui::executeImportWorkflow(bool _is_job_import_action) {

    /* CHECK UNSAVED ASSEMBLY OR JOB */

    bool confirm_action = true;

    if (!m_is_map_view) {
        if (m_is_assembly_modified || m_new_assembly) {
            confirm_action = promptAssemblyNotSaved();
        }
    }

    if (m_is_map_view && m_job_parameter_modified) {
        promptJobNotSaved();
    }

    if (!confirm_action) {
        return;
    }

    QString assembly_name = "";
    if (_is_job_import_action) {
        if (!m_current_assembly) {
            /* Impossible case : assembly has to be selected to provide access to job import function,
             * if this happens, the method is called by wrong caller */
            qCritical() << "No assembly selected, impossible to import job";
            return;
        }

        assembly_name = m_current_assembly->name();
    }

    if (m_import_export_helper->executeImportWorkflow(_is_job_import_action, assembly_name)) {
        // reload assemly tree
        sl_assembliesReload();
    }
}


void MainGui::doFoldUnfoldParameters(bool _do_unfold, bool _is_explicit_action)
{
    if (_do_unfold) {
        qDebug() << "Unfolding parameters";
        m_parameters_dock->show();
        m_ui->_PB_parameterFold->setToolTip(tr("Fold parameters window"));
    } else {
        qDebug() << "Folding parameters";
        m_parameters_dock->hide();
        m_ui->_PB_parameterFold->setToolTip(tr("Unfold parameters window"));
    }

    if (!_is_explicit_action) {
        // force button state change and repaint if state change is not explicitely
        // triggered by the fold/unfold button
        m_ui->_PB_parameterFold->setIsUnfolded(_do_unfold);
    }
}


void MainGui::freezeJobUserAction(bool _freeze_p)
{
    if(_freeze_p){
        m_ui->_TRW_assemblies->setEnabled(false);
        m_ui->_MCB_controllBar->setSwitchModeButtonEnable(false);
    }else{
        m_ui->_TRW_assemblies->setEnabled(true);
        m_ui->_MCB_controllBar->setSwitchModeButtonEnable(true);
    }
}

void MainGui::sl_showApplicationMode(eApplicationMode _mode)
{
    qDebug() << "Start application in mode " << _mode;
    m_active_application_mode = _mode;
    loadStyleSheet(_mode);

    if (_mode == APP_CONFIG) {
        PreferencesDialog dialog(this, m_icon_factory, m_preferences);
        dialog.setWindowFlags(Qt::Dialog | Qt::FramelessWindowHint);
        hide();
        /* If preferences are validated, new preferences are persisted */
        if (dialog.exec() == QDialog::Accepted) {
            SystemDataManager::instance()->writeMatissePreferences(PREFERENCES_FILEPATH, *m_preferences);
            ProcessDataManager::instance()->loadArchivedJobs(m_job_helper->archivePath());
            updateLanguage(m_preferences->language());
            updatePreferredDatasetParameters();
        }

        // retour à l'écran d'accueil
        sl_goHome();
    } else {
        sl_clearAssembly();
        m_ui->_TRW_assemblies->clearSelection();
        m_ui->_TRW_assemblyInfo->clear();
        // On recharge pour n'afficher que les traitements utilisables pour le mode
        sl_assembliesReload();

        // Reset and hide ongoing process indicators
        resetOngoingProcessIndicators();
        emit si_processStopped(); // to reset wheel if previous job was frozen

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

void MainGui::sl_goHome()
{
    bool confirm_action = true;

    if (!m_is_map_view) {
        if (m_is_assembly_modified || m_new_assembly) {
            confirm_action = promptAssemblyNotSaved();
        }
    }

    if (m_is_map_view && m_job_parameter_modified) {
        promptJobNotSaved();
    }

    if (confirm_action) {
        m_new_assembly = NULL;
        m_current_assembly = NULL;
        m_current_job = NULL;

        hide();
        m_welcome_dialog->enableProgrammingMode(m_preferences->programmingModeEnabled());
        m_welcome_dialog->show();
    }
}

void MainGui::sl_show3DFileOnMainView(QString _filepath_p)
{
    m_data_viewer->invokeThreaded3DFileLoader(_filepath_p);
}

void MainGui::sl_autoAdd3DFileFromFolderOnMainView(QString _folderpath_p)
{
    m_data_viewer->autoAdd3DFileFromFolderOnMainView(_folderpath_p);
}

void MainGui::sl_addRasterFileToMap(QString _filepath_p)
{
    m_data_viewer->loadRasterFile(_filepath_p);
}

void MainGui::sl_addToLog(QString _loggin_text)
{
    m_ui->_TW_infoTabs->setCurrentIndex(1);
    m_ui->_QTE_loggingText->append(_loggin_text);
}

void MainGui::saveAssemblyAndReload(AssemblyDefinition *_assembly)
{
    /* Save assembly and associated parameters */
    QString assembly_name = _assembly->name();
    m_assembly_editor->getScene()->updateAssembly(_assembly);
    ProcessDataManager::instance()->writeAssemblyFile(_assembly, true);
    m_engine.parametersManager()->saveParametersValues(assembly_name, true);

    if (_assembly == m_new_assembly) {
        m_new_assembly->deleteLater();
        m_new_assembly = NULL;
    }

    m_is_assembly_modified = false;

    sl_assembliesReload();

    // re-sélection de l'assemblage enregistré
    QTreeWidgetItem *item = m_assemblies_items.value(assembly_name);
    if (item) {
        m_ui->_TRW_assemblies->setCurrentItem(item);
        item->setSelected(true);
        // affichage de l'assemblage dans la vue graphique
        displayAssembly(assembly_name);
    } else {
        qWarning() << "Saved assembly is not found in reloaded tree : " << assembly_name;

        // on désactive la vue graphique
        m_assembly_editor->getScene()->reset();
        m_assembly_editor->getGraphicsView()->setEnabled(false);
    }
}

void MainGui::sl_saveAssembly()
{
    if (m_new_assembly) {

        qDebug() << "Saving new assembly " << m_new_assembly->name();

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

        QString assembly_name = getActualNewAssemblyName();
        m_new_assembly->setName(assembly_name);
        saveAssemblyAndReload(m_new_assembly);
        m_context.setLastActionPerformed(SAVE_ASSEMBLY);
        enableActions();

    } else { // Save existing assembly


        QTreeWidgetItem *selected_item = m_ui->_TRW_assemblies->currentItem();
        if (!selected_item) {
            qCritical() << "Selected item unknown. Assembly cannot be saved";
            return;
        }

        QString assembly_name = getActualAssemblyOrJobName(selected_item);
        KeyValueList *props = m_assemblies_properties.value(assembly_name);

        if (!m_assembly_helper->promptForAssemblyProperties(assembly_name, props)) {
            // disable graphical view
            m_assembly_editor->getScene()->reset();
            m_assembly_editor->getGraphicsView()->setEnabled(false);

            return;
        }

        AssemblyDefinition *assembly = ProcessDataManager::instance()->getAssembly(assembly_name);
        saveAssemblyAndReload(assembly);
        m_context.setLastActionPerformed(SAVE_ASSEMBLY);
        enableActions();
    }
}



void MainGui::displayAssemblyProperties(AssemblyDefinition *_selected_assembly)
{
    GraphicalCharter &graph_chart = GraphicalCharter::instance();

    m_ui->_TRW_assemblyInfo->clear();

    /* allocate items */
    m_assembly_version_property_item = new QTreeWidgetItem();
    m_assembly_creation_date_property_item = new QTreeWidgetItem();
    m_assembly_author_property_item = new QTreeWidgetItem();
    m_assembly_comment_property_header_item = new QTreeWidgetItem();
    m_assembly_comment_property_item = new QTreeWidgetItem();

    m_assembly_comment_property_item_text = new QLabel(_selected_assembly->comment());
    m_assembly_comment_property_item_text->setObjectName("_LA_assemblyCommentProperty");
    m_assembly_comment_property_item_text->setWordWrap(true);

    /* assign text values */
    m_assembly_version_property_item->setText(0, tr("Version:"));
    m_assembly_version_property_item->setText(1, _selected_assembly->version());

    m_assembly_creation_date_property_item->setText(0, tr("Creation date:"));
    m_assembly_creation_date_property_item->setText(1, _selected_assembly->date());

    m_assembly_author_property_item->setText(0, tr("Author:"));
    m_assembly_author_property_item->setText(1, _selected_assembly->author());

    m_assembly_comment_property_header_item->setText(0, tr("Comment:"));

    m_assembly_comment_property_item_text->setText(_selected_assembly->comment());
    m_assembly_comment_property_item_text->setToolTip(_selected_assembly->comment()); // to ensure full comment display

    m_ui->_TRW_assemblyInfo->addTopLevelItem(m_assembly_version_property_item);
    m_ui->_TRW_assemblyInfo->addTopLevelItem(m_assembly_creation_date_property_item);
    m_ui->_TRW_assemblyInfo->addTopLevelItem(m_assembly_author_property_item);
    m_ui->_TRW_assemblyInfo->addTopLevelItem(m_assembly_comment_property_header_item);
    m_ui->_TRW_assemblyInfo->addTopLevelItem(m_assembly_comment_property_item);

    m_ui->_TRW_assemblyInfo->header()->resizeSection(0, graph_chart.dpiScaled(ASSEMBLY_PROPS_LABEL_WIDTH));
    m_assembly_comment_property_header_item->setFirstColumnSpanned(true);
    m_assembly_comment_property_item->setFirstColumnSpanned(true);
    m_ui->_TRW_assemblyInfo->setItemWidget(m_assembly_comment_property_item, 0, m_assembly_comment_property_item_text);
}

void MainGui::loadAssemblyParameters(AssemblyDefinition *_selected_assembly)
{
    m_assembly_helper->loadAssemblyParameters(_selected_assembly);
}

void MainGui::displayAssembly(QString _assembly_name) {

    m_ui->_TRW_assemblyInfo->clear();
    if(m_data_viewer) {
        m_data_viewer->clear();
    }

    m_current_job = NULL;

    AssemblyDefinition *selected_assembly = ProcessDataManager::instance()->getAssembly(_assembly_name);

    if (!selected_assembly) {
        qCritical() << QString("Assembly '%1' not found").arg(_assembly_name);
        return;
    }

    m_current_assembly = selected_assembly;

    if (m_is_map_view) {

        loadAssemblyParameters(selected_assembly);
        displayAssemblyProperties(selected_assembly);

    } else {

        m_engine.parametersManager()->restoreParametersDefaultValues();
        m_assembly_editor->loadAssembly(_assembly_name);
        m_engine.parametersManager()->loadParameters(_assembly_name, true);

    }
}

void MainGui::displayJob(QString _job_name, bool _force_reload)
{
    GraphicalCharter &graph_chart = GraphicalCharter::instance();

    m_ui->_TRW_assemblyInfo->clear();
    if(m_data_viewer) {
        m_data_viewer->clear();
    }

    m_current_assembly = NULL;

    bool already_selected = false;

    if (m_current_job) {
        if (_job_name == m_current_job->name()) {
            already_selected = true;
        }
    }

    if (already_selected && !_force_reload) {
        qDebug() << QString("Job '%1' already selected").arg(_job_name);
        return;
    }

    AssemblyDefinition *selected_assembly = NULL;

    ProcessDataManager* process_data_manager = ProcessDataManager::instance();

    if (!already_selected) {
        m_current_job = process_data_manager->getJob(_job_name);

        if (!m_current_job) {
            qCritical() << QString("No job definition file found for selected job '%1'").arg(_job_name);
            return;
        }
    }

    QString assembly_name = m_current_job->assemblyName();
    selected_assembly = process_data_manager->getAssembly(assembly_name);

    if (!selected_assembly) {
        qCritical() << QString("Assembly file not found for assembly name '%1' owning job 'not found'%2'").arg(assembly_name).arg(_job_name);
        return;
    }

    loadAssemblyParameters(selected_assembly);
    m_engine.parametersManager()->loadParameters(_job_name, false);

    /* DISPLAY JOB PROPERTIES AND RESULT IMAGES */
    QString comments = m_current_job->comment();

    QTreeWidgetItem *job_comment_label_item = NULL;
    QTreeWidgetItem *job_comment_item = NULL;
    QLabel *jobCommentItemText = NULL;

    if (comments != "") {
        job_comment_label_item = new QTreeWidgetItem();
        job_comment_label_item->setText(0, tr("Comment:"));
        m_ui->_TRW_assemblyInfo->addTopLevelItem(job_comment_label_item);

        job_comment_item = new QTreeWidgetItem();
        m_ui->_TRW_assemblyInfo->addTopLevelItem(job_comment_item);

        jobCommentItemText = new QLabel();
        jobCommentItemText->setObjectName("_LA_jobCommentProperty");
        jobCommentItemText->setText(comments);
        jobCommentItemText->setToolTip(comments);
        jobCommentItemText->setWordWrap(true);
    }

    QTreeWidgetItem *job_result_label_item = NULL;
    QTreeWidgetItem **job_result_image_items = NULL;
    int nb_of_image_files = 0;

    if (m_current_job->executionDefinition() && m_current_job->executionDefinition()->executed()) {
        QTreeWidgetItem *job_execution_date_item = new QTreeWidgetItem();
        job_execution_date_item->setText(0, tr("Execution date:"));
        QString execution_date_str = m_current_job->executionDefinition()->executionDate().toString(tr("dd/MM/yyyy HH:mm"));
        job_execution_date_item->setText(1, execution_date_str);
        m_ui->_TRW_assemblyInfo->addTopLevelItem(job_execution_date_item);

        job_result_label_item = new QTreeWidgetItem();
        job_result_label_item->setText(0, tr("Result file:"));
        m_ui->_TRW_assemblyInfo->addTopLevelItem(job_result_label_item);


        QStringList result_images = m_current_job->executionDefinition()->resultFileNames();
        nb_of_image_files = result_images.size();

        job_result_image_items = new QTreeWidgetItem *[nb_of_image_files];

        // display result
        QMessageBox::StandardButton reply;
        reply = QMessageBox::question(this, tr("Load result ?"), tr("Do you want to load result in Matisse (can take time for big reconstructions) ?"),
                                      QMessageBox::Yes|QMessageBox::No);

        for (int i = 0 ; i < nb_of_image_files ; i++) {

            QString result_file = result_images[i];

            QTreeWidgetItem *job_result_item = new QTreeWidgetItem();
            job_result_item->setText(0, result_file);
            job_result_item->setToolTip(0, result_file);
            m_ui->_TRW_assemblyInfo->addTopLevelItem(job_result_item);
            job_result_image_items[i] = job_result_item;

            if (result_file.isEmpty()) {
                qCritical() << "Job marked as executed but result filename is empty";
                continue;
            }

            if (reply == QMessageBox::Yes)
                loadResultToCartoView(result_file, false);
        }
    }

    /* resize first column */
    m_ui->_TRW_assemblyInfo->header()->resizeSection(0, graph_chart.dpiScaled(ASSEMBLY_PROPS_LABEL_WIDTH));
    if (job_comment_item) {
        job_comment_label_item->setFirstColumnSpanned(true);
        job_comment_item->setFirstColumnSpanned(true);
        /* allow multiline comment display */
        m_ui->_TRW_assemblyInfo->setItemWidget(job_comment_item, 0, jobCommentItemText);
    }

    if (job_result_label_item) {
        job_result_label_item->setFirstColumnSpanned(true);

        for (int i = 0 ; i < nb_of_image_files ; i++) {
            QTreeWidgetItem *item = job_result_image_items[i];
            item->setFirstColumnSpanned(true);
        }
    }

    /* memory deallocation for the array of QTreeWidgetItem* */
    delete[] job_result_image_items;
}

void MainGui::selectAssembly(QString _assembly_name, bool _reload_assembly)
{
    bool found = false;

    for (int index_assembly = 0; index_assembly < m_ui->_TRW_assemblies->topLevelItemCount(); index_assembly++) {
        QTreeWidgetItem * assembly_item = m_ui->_TRW_assemblies->topLevelItem(index_assembly);

        QString current_assembly_actual_name = getActualAssemblyOrJobName(assembly_item);

        if (current_assembly_actual_name == _assembly_name) {
            // selecting item
            found = true;
            m_ui->_TRW_assemblies->setCurrentItem(assembly_item);

            if (_reload_assembly) {
                displayAssembly(_assembly_name);
            }

            break;
        }
    }

    if (!found) {
        qCritical() << QString("Assembly '%1' was not found, selection impossible.").arg(_assembly_name);
    }
}

void MainGui::selectJob(QString _job_name, bool _reload_job)
{
    bool found = false;

    // on parcourt l'arbre à  la recherche du job
    for (int index_assembly = 0; index_assembly < m_ui->_TRW_assemblies->topLevelItemCount(); index_assembly++) {
        QTreeWidgetItem * assembly_item = m_ui->_TRW_assemblies->topLevelItem(index_assembly);
        for (int index_job = 0; index_job < assembly_item->childCount(); index_job++) {
            QTreeWidgetItem * job_item = assembly_item->child(index_job);
            QString current_job_actual_name = getActualAssemblyOrJobName(job_item);

            if (current_job_actual_name == _job_name) {
                // selection de l'item...
                found = true;
                m_ui->_TRW_assemblies->setCurrentItem(job_item);
                if (_reload_job) {
                    displayJob(_job_name);
                }
                break;
            }
        }
        if (found)
            break;
    }

    if (!found) {
        qCritical() << QString("Job '%1' was not found, selection impossible.").arg(_job_name);
    }
}


void MainGui::deleteAssemblyAndReload(bool _prompt_user)
{
    QTreeWidgetItem * selected_item = m_ui->_TRW_assemblies->currentItem();
    if (!selected_item) {
        qCritical() << "No item was selected";
        return;
    }

    if (selected_item->childCount() > 0) {
        // jobs exist... removal prohibited
        // this case should not occur : controlled at contextual menu creation
        qCritical() << "Jobs are attached to the assembly, it cannot be removed.";
        return;
    }

    QString assembly_name = getActualAssemblyOrJobName(selected_item);

    // Check for archived jobs and prompt user
    if (!m_assembly_helper->checkBeforeAssemblyDeletion(assembly_name, _prompt_user)) {
        return;
    }

    // Handle deletion of a newly created assembly
    if (m_new_assembly) {
        QString new_assembly_name = getActualNewAssemblyName();

        if (new_assembly_name == assembly_name) { // ensure assembly selected for deletion is the new assembly
            qDebug() << QString("Removing new assembly '%1'").arg(assembly_name);
            m_new_assembly->deleteLater();
            m_new_assembly = NULL;
            sl_assembliesReload();

            // Reactivate Assembly->Create menu item
            m_create_assembly_act->setEnabled(true);

            return;
        } else {
            // this case should not occur because it is handled at selection
            qCritical() << QString("A new assembly '%1' was not saved, assembly '%2' cannot be deleted").arg(new_assembly_name).arg(assembly_name);
            return;
        }
    }

    // Handle deletion of a registered assembly
    if (m_assembly_helper->deleteAssembly(assembly_name)) {
        sl_assembliesReload();
    }
}



void MainGui::sl_deleteAssembly()
{
    qDebug() << "Delete selected assembly";

    deleteAssemblyAndReload(true);
}

void MainGui::sl_newJob()
{
    // fold parameters widget so the dialog is displayed on top of the dark background
    doFoldUnfoldParameters(false);

    QTreeWidgetItem * item = m_ui->_TRW_assemblies->currentItem();
    if (!item) {
        qCritical() << "No assembly selected !";
        return;
    }

    QString assembly_name = item->text(0);

    JobDefinition *new_job_def = m_job_helper->createNewJob(assembly_name);

    if (!new_job_def) {
        // Job could not be created
        return;
    }

    // display and select new job (flag is set to false because job was already saved)
    if (addJobInTree(new_job_def, false)) {
        selectJob(new_job_def->name());
    }

    m_engine.parametersManager()->toggleReadOnlyMode(false); // enable parameters editing
    doFoldUnfoldParameters(true);

    m_context.setLastActionPerformed(SAVE_JOB);
    enableActions();
}

void MainGui::sl_saveJob()
{
    qDebug() << "Save current job";

    QTreeWidgetItem * item = m_ui->_TRW_assemblies->currentItem();
    if (!item) {
        qCritical() << "No job selected";
        return;
    }

    if (!m_current_job) {
        qWarning() << "Current job not identified";

        QString job_name = getActualAssemblyOrJobName(item);
        m_current_job = ProcessDataManager::instance()->getJob(job_name);

        if (!m_current_job) {
            qCritical() << QString("Selected job '%1' not found in local repository").arg(job_name);
            return;
        }
    }

    bool already_executed = m_current_job->executionDefinition()->executed();
    bool job_saved = m_job_helper->saveJob(m_current_job);

    // update status if job was executed prior to updating
    if (already_executed) {
        if (item->type() == MatisseTreeItem::Type) {
            MatisseTreeItem *job_item = static_cast<MatisseTreeItem *>(item);
            IconizedWidgetWrapper *item_wrapper = new IconizedTreeItemWrapper(job_item, 0);
            m_icon_factory->attachIcon(item_wrapper, "lnf/icons/led.svg", false, false, m_colors_by_level.value(IDLE));
        } else {
            qCritical() << QString("Item for job '%1' is not of type MatisseTreeItem, cannot display icon").arg(m_current_job->name());
        }

        m_data_viewer->clear();
    }

    if (!job_saved) {
        return;
    }

    // reinit flag and job displayed name
    m_job_parameter_modified = false;
    handleJobModified();

    // re-select job
    displayJob(m_current_job->name(), true);
}


void MainGui::sl_deleteJob()
{
    qDebug() << "Delete current job";
    QTreeWidgetItem * selected_item = m_ui->_TRW_assemblies->currentItem();
    if (!selected_item) {
        // nothing selected : inconsistent
        return;
    }
    if (selected_item->parent() == NULL) {
        // assembly selected : inconsistent
        return;
    }

    QString job_name = getActualAssemblyOrJobName(selected_item);

    if (!m_job_helper->deleteJob(job_name)) {
        return;
    }

    m_data_viewer->resetJobForm();
    // message effacement OK
    // suppression liste: rechargement...
    sl_assembliesReload();
    doFoldUnfoldParameters(false);
}

void MainGui::sl_assemblyContextMenuRequested(const QPoint &_pos)
{
    QTreeWidgetItem* item = m_ui->_TRW_assemblies->itemAt(_pos);

    // Select in case left click was not done before
    sl_selectAssemblyOrJob(item);

    if (!item) {
        qWarning() << "Right click on assembly tree widget : no item selected";
        return;
    }

    QMenu* context_menu = new QMenu(m_ui->_TRW_assemblies);

    if (!item->parent()) { // menu contextuel pour traitement
        context_menu->addAction(m_create_job_act);
        context_menu->addAction(m_import_job_act);
        context_menu->addAction(m_clone_assembly_act);
        // Assemblies cannot be deleted if they have jobs attached
        if (item->childCount() == 0) {
            context_menu->addAction(m_delete_assembly_act);
        }

        QString assembly_name = item->text(0);
        if (ProcessDataManager::instance()->assemblyHasArchivedJob(assembly_name)) {
            context_menu->addAction(m_restore_job_act);
        }

        context_menu->addAction(m_update_assembly_properties_act);

    } else {              // menu contextuel pour tâche
        if (!m_current_job) {
            qCritical() << "Activated context menu on job but current job is unavailable";
            return;
        }

        context_menu->addAction(_executeJobAct);
        if (m_current_job->executionDefinition()->executed()) {
            // show only if job was executed
            context_menu->addAction(m_go_to_results_act);
        }
        context_menu->addSeparator();
        context_menu->addAction(m_upload_data_act);
        context_menu->addAction(m_select_remote_data_act);
        context_menu->addAction(m_execute_remote_job_act);
        if (m_current_job->remoteJobDefinition()->isScheduled()) {
          // show only if job was scheduled on remote server
          context_menu->addAction(m_download_job_results_act);
        }
        context_menu->addSeparator();
        context_menu->addAction(m_save_job_act);
        context_menu->addAction(m_clone_job_act);
        context_menu->addAction(m_export_job_act);
        context_menu->addAction(m_delete_job_act);
        context_menu->addAction(m_archive_job_act);
    }

    context_menu->popup(m_ui->_TRW_assemblies->viewport()->mapToGlobal(_pos));

}

QTreeWidgetItem *MainGui::addAssemblyInTree(AssemblyDefinition * _assembly)
{
    QTreeWidgetItem * assembly_item = new QTreeWidgetItem(QStringList() << _assembly->name());
    assembly_item->setData(0, Qt::UserRole,_assembly->filename());
    m_ui->_TRW_assemblies->addTopLevelItem(assembly_item);
    m_assemblies_items.insert(_assembly->name(), assembly_item);

    KeyValueList *props = new KeyValueList();
    props->insert("Version", _assembly->version());
    props->insert("Valid", QString::number(_assembly->usable()));
    props->insert("Author", _assembly->author());
    props->insert("Comments", _assembly->comment());

    /* if new assembly (suffixed by '*'), properties are indexed by the actual assembly name */
    QString actual_assembly_name = getActualAssemblyOrJobName(assembly_item);
    m_assemblies_properties.insert(actual_assembly_name, props);

    return assembly_item;
}

QTreeWidgetItem *MainGui::addJobInTree(JobDefinition * _job, bool _is_new_job)
{
    bool executed = false;

    QString assembly = _job->assemblyName();

    if (_job->executionDefinition() == NULL) {
        qWarning() << "Job definition not found";
        executed = false;
    } else {
        executed = _job->executionDefinition()->executed();
    }

    QTreeWidgetItem * assembly_item = m_assemblies_items.value(assembly, NULL);
    if (!assembly_item) {
        // L'assemblage n'existe pas...
        return NULL;
    }

    QString job_name = _job->name();

    if (_is_new_job) {
        job_name.append("*");
    }

    MatisseTreeItem *job_item = new MatisseTreeItem(assembly_item, QStringList() << job_name);
    job_item->setData(0, Qt::UserRole, assembly);
    IconizedWidgetWrapper *item_wrapper = new IconizedTreeItemWrapper(job_item, 0);

    if (executed) {
        m_icon_factory->attachIcon(item_wrapper, "lnf/icons/led.svg", false, false, m_colors_by_level.value(OK));

    } else {
        m_icon_factory->attachIcon(item_wrapper, "lnf/icons/led.svg", false, false, m_colors_by_level.value(IDLE));
    }

    return job_item;
}

void MainGui::initStatusBar()
{
    m_status_message_widget = new StatusMessageWidget(this, m_icon_factory);
    m_status_message_widget->setObjectName("_WID_statusMessage");
    statusBar()->addPermanentWidget(m_status_message_widget, 10);
}

void MainGui::showStatusMessage(QString _message, eJobStatusMessageLevel _level)
{
    if (_message.isEmpty()) {
        return;
    }

    QString level_color_alias = m_colors_by_level.value(_level);
    m_status_message_widget->addMessage(_message, "lnf/icons/led.svg", level_color_alias);

    emit si_updateExecutionStatusColor(m_colors_by_level.value(_level));
}


void MainGui::initLanguages()
{
    m_server_translator_en = new QTranslator();
    m_server_translator_en->load("i18n/MatisseServer_en");

    m_server_translator_fr = new QTranslator();
    m_server_translator_fr->load("i18n/MatisseServer_fr");

    m_tools_translator_en = new QTranslator();
    m_tools_translator_en->load("i18n/MatisseTools_en");

    m_tools_translator_fr = new QTranslator();
    m_tools_translator_fr->load("i18n/MatisseTools_fr");

    // Langue par défaut : Français
    m_current_language = "FR";
    qApp->installTranslator(m_server_translator_fr);
    qApp->installTranslator(m_tools_translator_fr);
}

void MainGui::updateLanguage(QString _language, bool _force_retranslation)
{
    if (_language == m_current_language) {
        if (_force_retranslation) {
            retranslate();
        } else {
            qDebug("No language change");
        }
        return;
    }

    m_current_language = _language;

    if (_language == "EN") {
        qDebug() << "Translating UI to English";

        qApp->removeTranslator(m_server_translator_fr);
        qApp->removeTranslator(m_tools_translator_fr);
        qApp->installTranslator(m_server_translator_en);
        qApp->installTranslator(m_tools_translator_en);
    } else {
        qDebug() << "Restoring UI to French";

        qApp->removeTranslator(m_server_translator_en);
        qApp->removeTranslator(m_tools_translator_en);
        qApp->installTranslator(m_server_translator_fr);
        qApp->installTranslator(m_tools_translator_fr);
    }
}

void MainGui::retranslate()
{
    qDebug() << "Translating static and contextual menu items...";

    /* MENU FICHIER */
    m_file_menu->setTitle(tr("FILE"));
    m_export_map_view_act->setText(tr("Export view to image"));
    m_close_act->setText(tr("Close"));

    /* MENU AFFICHAGE */
    m_display_menu->setTitle(tr("DISPLAY"));
    m_day_night_mode_act->setText(tr("Day/night mode"));
    m_map_toolbar_act->setText(tr("Toolbar"));

    /* MENU TRAITEMENTS */
    m_process_menu->setTitle(tr("PROCESSING"));
    m_create_assembly_act->setText(tr("Create"));
    m_save_assembly_act->setText(tr("Save"));
    m_import_assembly_act->setText(tr("Import"));
    m_export_assembly_act->setText(tr("Export"));

    /* MENU OUTILS */
    m_tool_menu->setTitle(tr("TOOLS"));
    m_app_config_act->setText(tr("Configure settings for application"));
    m_preprocessing_tool->setText(tr("Launch preprocessing tool"));
    m_nmea_nav_extrator_tool->setText(tr("Launch nmea nav extrator"));
    m_camera_manager_tool->setText(tr("Launch camera manager"));
    m_camera_calib_tool->setText(tr("Launch camera calibration tool"));

    /* MENU AIDE */
    m_help_menu->setTitle(tr("HELP"));
    m_user_manual_act->setText(tr("User manual"));
    m_about_act->setText(tr("About"));


    /* Menu contextuel Traitement */
    m_create_job_act->setText(tr("Create new task"));
    m_import_job_act->setText(tr("Import task"));
    m_clone_assembly_act->setText(tr("Copy"));
    m_delete_assembly_act->setText(tr("Delete processing chain"));
    m_restore_job_act->setText(tr("Restore"));
    m_update_assembly_properties_act->setText(tr("Update properties"));

    /* Menu contextuel Tâche */
    _executeJobAct->setText(tr("Run"));
    m_upload_data_act->setText(tr("Upload data to server"));
    m_select_remote_data_act->setText(tr("Select dataset on server"));
    m_execute_remote_job_act->setText(tr("Run on server"));
    m_download_job_results_act->setText(tr("Download results"));
    m_save_job_act->setText(tr("Save"));
    m_clone_job_act->setText(tr("Copy"));
    m_export_job_act->setText(tr("Export"));
    m_delete_job_act->setText(tr("Delete"));
    m_archive_job_act->setText(tr("Archive"));
    m_go_to_results_act->setText(tr("Open reconstruction folder"));
}


// Dynamic translation
void MainGui::changeEvent(QEvent *_event)
{
    if (_event->type() == QEvent::LanguageChange)
    {
        // retranslate designer form
        qDebug() << "Retranslating UI...";
        m_ui->retranslateUi(this);
        retranslate();
    }
}

void MainGui::sl_maximizeOrRestore()
{
    if (isMaximized()) {
        showNormal();
        m_icon_factory->attachIcon(m_max_or_restore_button_wrapper, "lnf/icons/agrandir.svg", false, false);
        m_maximize_or_restore_button->setToolTip(tr("Maximize main window"));
    } else {
        showMaximized();
        m_icon_factory->attachIcon(m_max_or_restore_button_wrapper, "lnf/icons/reinittaille.svg", false, false);
        m_maximize_or_restore_button->setToolTip(tr("Restore window size"));
    }
}

void MainGui::sl_quit()
{
    bool confirm_action = true;

    if (!m_is_map_view) {
        if (m_is_assembly_modified || m_new_assembly) {
            confirm_action = promptAssemblyNotSaved();
        }
    }

    if (m_is_map_view && m_job_parameter_modified) {
        promptJobNotSaved();
    }

    if (confirm_action) {
        close();
    }
}

void MainGui::sl_moveWindow(const QPoint &_pos)
{
    move(_pos);
}

void MainGui::sl_clearAssembly()
{
    m_ui->_LW_inputs->clearSelection();
    m_ui->_LW_processors->clearSelection();
    m_ui->_LW_outputs->clearSelection();

    m_assembly_editor->resetAssemblyForm();
    m_ui->_TW_creationViewTabs->setCurrentIndex(0);
}

void MainGui::sl_newAssembly()
{
    qDebug() << "Creating new assembly...";

    // fold parameters widget so the dialog is displayed on top of the dark background
    doFoldUnfoldParameters(false);

    sl_clearAssembly();

    m_new_assembly = m_assembly_helper->createNewAssembly();
    if (!m_new_assembly) {
        return;
    }

    QTreeWidgetItem *item = addAssemblyInTree(m_new_assembly);

    m_ui->_TRW_assemblies->setCurrentItem(item);
    item->setSelected(true);

    m_current_assembly = NULL;

    displayAssemblyProperties(m_new_assembly);

    // unfold parameters
    doFoldUnfoldParameters(true);

    m_context.setLastActionPerformed(CREATE_ASSEMBLY);
    enableActions();

    // activate graphical view
    m_assembly_editor->getGraphicsView()->setEnabled(true);

}

void MainGui::enableActions()
{
    eUserAction last_action = m_context.lastActionPerformed();

    /* enable/diable main menu items */
    m_export_map_view_act->setEnabled(m_is_map_view);
    m_map_toolbar_act->setEnabled(m_is_map_view);
    m_preprocessing_tool->setEnabled(m_is_map_view);
    m_nmea_nav_extrator_tool->setEnabled(m_is_map_view);
    m_camera_manager_tool->setEnabled(m_is_map_view);
    m_camera_calib_tool->setEnabled(m_is_map_view);

    if (last_action == SELECT_ASSEMBLY || last_action == SAVE_ASSEMBLY) {
        m_export_assembly_act->setEnabled(true);
    } else {
        m_export_assembly_act->setEnabled(false);
    }

    if (last_action == CREATE_ASSEMBLY) {
        m_create_assembly_act->setEnabled(false);
        m_save_assembly_act->setEnabled(false);
        m_import_assembly_act->setEnabled(false);

        m_clone_assembly_act->setVisible(false);
        m_update_assembly_properties_act->setVisible(false);
    } else if (last_action == MODIFY_ASSEMBLY){
        m_create_assembly_act->setEnabled(false);
        m_save_assembly_act->setEnabled(m_is_assembly_complete);
        m_import_assembly_act->setEnabled(false);

        m_clone_assembly_act->setVisible(false);
        m_update_assembly_properties_act->setVisible(false);
    } else {
        m_create_assembly_act->setEnabled(!m_is_map_view);
        m_save_assembly_act->setEnabled(false);
        m_import_assembly_act->setEnabled(true);

        m_clone_assembly_act->setVisible(!m_is_map_view);
        m_update_assembly_properties_act->setVisible(!m_is_map_view);
    }

    /* show / hide assembly contextual menu items */
    m_create_job_act->setVisible(m_is_map_view);
    m_import_job_act->setVisible(m_is_map_view);
    m_restore_job_act->setVisible(m_is_map_view);
    m_delete_assembly_act->setVisible(!m_is_map_view);

    /* Job contextual menu items are always active as jobs are only visible in the map view */
}

SourceWidget *MainGui::getSourceWidget(QString _name)
{
    SourceWidget * wid = m_available_sources.value(_name, 0);

    if (!wid) {
        return 0;
    }

    m_engine.addParametersForInputDataProvider(_name);

    SourceWidget * new_widget = new SourceWidget();
    new_widget->clone(wid);

    return new_widget;
}

ProcessorWidget *MainGui::getProcessorWidget(QString _name)
{
    ProcessorWidget * wid = m_available_processors.value(_name, 0);

    if (!wid) {
        return 0;
    }

    m_engine.addParametersForProcessor(_name);

    ProcessorWidget * new_widget = new ProcessorWidget();
    new_widget->clone(wid);

    return new_widget;
}

DestinationWidget *MainGui::getDestinationWidget(QString _name)
{
    DestinationWidget * wid = m_available_destinations.value(_name, 0);

    if (!wid) {
        return 0;
    }

    m_engine.addParametersForOutputDataWriter(_name);

    DestinationWidget * new_widget = new DestinationWidget();
    new_widget->clone(wid);

    return new_widget;
}


void MainGui::applyNewApplicationContext()
{
    bool is_expert = (m_active_application_mode == PROGRAMMING);
    m_engine.parametersManager()->applyApplicationContext(is_expert, !m_is_map_view);
}

void MainGui::sl_swapMapOrCreationView()
{
    qDebug() << "Current view displayed : " << ((m_is_map_view) ? "map" : "creation");

    if (m_is_map_view) {
        if (m_job_parameter_modified) {
            promptJobNotSaved();
        }

        qDebug() << "Swapping to creation view";

        m_is_map_view = false;
        if (m_active_application_mode == PROGRAMMING) {
            m_active_view_or_mode_label->setText(tr("View : Creation"));
        }
        //        _visuModeButton->setIcon(_creationVisuModeIcon);
        m_icon_factory->attachIcon(m_visu_mode_button_wrapper, "lnf/icons/Clef.svg", false, false);
        m_visu_mode_button->setToolTip(tr("Switch view to cartography"));

        // swap view (1: creation view)
        m_ui->_SW_viewStack->setCurrentIndex(1);

        m_ui->_TRW_assemblies->collapseAll();
        m_ui->_TRW_assemblies->setItemsExpandable(false);
        sl_clearAssembly();
        // on désactive la vue graphique initialement
        m_assembly_editor->getGraphicsView()->setEnabled(false);
        m_ui->_TW_creationViewTabs->setCurrentIndex(0);

        // enable parameters editing
        m_engine.parametersManager()->toggleReadOnlyMode(false);

    } else {
        if (m_is_assembly_modified || m_new_assembly) {
            bool confirm_action = promptAssemblyNotSaved();

            if (!confirm_action) {
                return;
            }

            m_is_assembly_modified = false;
            handleAssemblyModified();
        }

        qDebug() << "Swapping to map view";
        m_is_map_view = true;
        if (m_active_application_mode == PROGRAMMING) {
            m_active_view_or_mode_label->setText(tr("View : Cartography"));
        }
        //        _visuModeButton->setIcon(_mapVisuModeIcon);
        m_icon_factory->attachIcon(m_visu_mode_button_wrapper, "lnf/icons/Cartographie.svg", false, false);
        m_visu_mode_button->setToolTip(tr("Switch view to Creation"));

        /* Reset visible parameters */
        m_engine.parametersManager()->clearExpectedParameters();

        // swap view (0: map view)
        m_ui->_SW_viewStack->setCurrentIndex(0);

        // Changement des info bulles
        m_ui->_TRW_assemblies->setItemsExpandable(true);
        m_ui->_TRW_assemblies->expandAll();
        m_data_viewer->resetJobForm();
        m_ui->_TW_infoTabs->setCurrentIndex(0);
    }

    m_new_assembly = NULL;
    m_current_assembly = NULL;
    m_current_job = NULL;

    m_ui->_TRW_assemblies->clearSelection();

    /* select tab set */
    m_ui->_SW_helperTabSets->setCurrentIndex(!m_is_map_view);

    /* hide live process indicators */
    resetOngoingProcessIndicators();

    // Apply new context to parameters widget
    applyNewApplicationContext();
    doFoldUnfoldParameters(false);

    m_context.setLastActionPerformed(SWAP_VIEW);
    enableActions();

}


void MainGui::sl_launchJob()
{
    qDebug() << "Launching job...";

    QTreeWidgetItem * current_item = m_ui->_TRW_assemblies->currentItem();

    if (!current_item) {

        qCritical() << "No job selected, impossible to launch";
        return;
    }

    QTreeWidgetItem * parent_item = current_item->parent();

    if (!parent_item) {
        qCritical() << "An assembly was selected, cannot run job";
        return;
    }

    QString job_name;

    if (!m_current_job) {
        qWarning() << "Current job not identified";

        job_name = getActualAssemblyOrJobName(current_item);

        m_current_job = ProcessDataManager::instance()->getJob(job_name);

        if (!m_current_job) {
            qCritical() << QString("Selected job '%1' not found in local repository, impossible to launch").arg(job_name);
            return;
        }
    } else {
        job_name = m_current_job->name();
    }


    if (m_current_job->executionDefinition()->executed()) {
        if (QMessageBox::No == QMessageBox::question(this, tr("Job already processed..."),
                                                     tr("Job already processed.\n Process again ?"),
                                                     QMessageBox::Yes,
                                                     QMessageBox::No)) {
            return;
        }
        m_data_viewer->clear();
    }

    // If a parameter was modified, the user is prompted for saving parameter values
    if (m_job_parameter_modified) {
        if (QMessageBox::No == QMessageBox::question(this, tr("Parameters changed..."),
                                                     tr("One or more parameters were modified.\nDo you want to save task parameters ?"),
                                                     QMessageBox::Yes,
                                                     QMessageBox::No)) {

            qDebug() << "User aborted job execution";
            return;
        } else {
            qDebug() << "Saving job parameters before launch";
            // Save parameter values
            m_engine.parametersManager()->saveParametersValues(job_name, false);
            m_job_parameter_modified = false;
            handleJobModified();
        }
    }

    QString status_msg;
    bool job_ready = m_job_helper->prepareJobForExecution(m_current_job, status_msg);
    // reloading pointer to current job from local cache (mandatory since
    // JobHelper reloaded object from file)
    m_current_job = ProcessDataManager::instance()->getJob(job_name);

    eJobStatusMessageLevel msg_level = (job_ready) ? IDLE : ERR;
    showStatusMessage(status_msg, msg_level);

    if (!job_ready) {
        return;
    }

    m_last_job_launched_item = current_item;

    if (m_last_job_launched_item->type() == MatisseTreeItem::Type) {
        MatisseTreeItem *last_job_launched_icon_item = static_cast<MatisseTreeItem *>(m_last_job_launched_item);
        IconizedWidgetWrapper *item_wrapper = new IconizedTreeItemWrapper(last_job_launched_icon_item, 0);
        m_icon_factory->attachIcon(item_wrapper, "lnf/icons/led.svg", false, false, m_colors_by_level.value(WARNING));
    } else {
        qWarning() << QString("Item for job '%1' is not of type MatisseTreeItem, cannot display icon").arg(job_name);
    }

    m_ui->_QTE_loggingText->clear(); // clear job log
    doFoldUnfoldParameters(false);
    m_stop_button->setEnabled(true);
    m_ongoing_process_info_label->show();
    m_ongoing_process_completion->show();
    emit si_processRunning();

    // launch job
    bool run_success = m_engine.processJob(m_current_job);

    if (!run_success) {
        QString msg = tr("Error %1: %2").arg(job_name).arg(m_engine.messageStr());
        showStatusMessage(msg, ERR);
        m_stop_button->setEnabled(false);
    }else{
        freezeJobUserAction(true);
    }
}


void MainGui::sl_launchRemoteJob()
{
    qDebug() << "Launching remote job...";

    executeExportWorkflow(true, true);
    m_remote_job_helper->scheduleJob(m_current_job->name(), m_current_remote_execution_bundle);
}


void MainGui::sl_uploadJobData()
{
    if (!m_current_job) {
      qCritical() << "No job selected, cannot upload job data";
      return;
    }

    QString job_name = m_current_job->name();

    m_remote_job_helper->uploadDataset(job_name);
}

void MainGui::sl_selectRemoteJobData()
{
  if (!m_current_job) {
    qCritical() << "No job selected, cannot select remote job data";
    return;
  }

  QString job_name = m_current_job->name();

  m_remote_job_helper->selectRemoteDataset(job_name);
}

void MainGui::sl_downloadJobResults()
{
  if (!m_current_job) {
    qCritical() << "No job selected, cannot download results";
    return;
  }

  QString job_name = m_current_job->name();

  m_remote_job_helper->downloadResults(job_name);
}

void MainGui::sl_onRemoteJobResultsReceived(
    QString _job_name) 
{
  /* Reselect job */
  selectJob(_job_name);

  /* Update status */
  QTreeWidgetItem *job_item = m_ui->_TRW_assemblies->currentItem();
  updateJobStatus(_job_name, job_item, OK, tr("Job %1 executed on remote server..."));

  /* Reload job and display results */
  displayJob(_job_name, true);
}

void MainGui::sl_stopJob()
{
    QString job_name = m_last_job_launched_item->data(0, Qt::UserRole).toString();
    qDebug() << "Stopping job " << job_name.toLatin1();

    QString msg1=tr("Stopping running job.");
    QString msg2=tr("Do you want to stop or cancel the job?");
    QMessageBox msg_box;
    msg_box.setText(msg1);
    msg_box.setInformativeText(msg2);
    QPushButton *stop_button = msg_box.addButton(tr("Stop"), QMessageBox::AcceptRole);
    QPushButton *discard_button = msg_box.addButton(QMessageBox::Discard);
    msg_box.addButton(QMessageBox::Cancel);

    msg_box.exec();
    if (msg_box.clickedButton() == stop_button) {
        m_engine.stopJob(false);

        resetOngoingProcessIndicators();
        emit si_processStopped();
        m_stop_button->setEnabled(false);
    }
    else if (msg_box.clickedButton() == discard_button) {
        m_engine.stopJob(true);

        resetOngoingProcessIndicators();
        emit si_processStopped();
        m_stop_button->setEnabled(false);
    }
    else {
        // cancel dialog => do nothing
    }

    freezeJobUserAction(false);

}

void MainGui::sl_jobShowImageOnMainView(QString _name, Image *_image)
{
    Q_UNUSED(_name)
    if (_image) {
        m_data_viewer->displayImage(_image);
        delete _image;
    }
}

void MainGui::sl_userInformation(QString _user_text)
{
    //qDebug() << "Received user information : " << userText;
    m_ongoing_process_info_label->setText(_user_text);
}

void MainGui::sl_processCompletion(quint8 _percent_complete)
{
    //qDebug() << "Received process completion signal : " << percentComplete;

    if(_percent_complete == (quint8)-1)
    {
        m_ongoing_process_completion->setTextVisible(false);
        m_ongoing_process_completion->setValue(0);
    }
    else
    {
        m_ongoing_process_completion->setTextVisible(true);

        if (_percent_complete > 100) {
            qWarning() << QString("Invalid process completion percentage value : %1").arg(_percent_complete);
            return;
        }

        m_ongoing_process_completion->setValue(_percent_complete);
    }
}

void MainGui::sl_showInformationMessage(QString _title, QString _message)
{
    QMessageBox::information(this, _title, _message);
}

void MainGui::sl_showErrorMessage(QString _title, QString _message)
{
    QMessageBox::critical(this, _title, _message);
}


void MainGui::sl_jobProcessed(QString _job_name, bool _is_cancelled) {

    if (!m_job_helper->jobProcessed(_job_name)) {
        return;
    }

    JobDefinition *job_def = ProcessDataManager::instance()->getJob(_job_name);

    if (m_last_job_launched_item) {
        if (_is_cancelled) {
            updateJobStatus(_job_name, m_last_job_launched_item, IDLE, tr("Job %1 cancelled..."));

            // TODO Image cleaning ?
        }
        else  {
            updateJobStatus(_job_name, m_last_job_launched_item, OK, tr("Job %1 finished..."));

            selectJob(job_def->name());
        }
    }

    QMessageBox::StandardButton reply;
    reply = QMessageBox::question(this, tr("Load result ?"), tr("Do you want to load result in Matisse (can take time for big reconstructions) ?"),
        QMessageBox::Yes | QMessageBox::No);


    foreach (QString result_file, job_def->executionDefinition()->resultFileNames()) {

        if (job_def->executionDefinition()->executed() && (!result_file.isEmpty())) {

            // display result
            if (reply == QMessageBox::Yes)
                loadResultToCartoView(result_file, false);
        }
    }

    // Désactiver le bouton STOP
    emit si_processStopped();
    m_stop_button->setEnabled(false);
    freezeJobUserAction(false);
}

void MainGui::sl_assembliesReload()
{
    loadAssembliesAndJobsLists(m_is_map_view);

    if (m_is_map_view) {
        m_data_viewer->clear();
        //qDebug() << "Clear userForm...";
    } else {
        sl_clearAssembly();
    }

    m_engine.parametersManager()->clearExpectedParameters();
    m_current_assembly = NULL;
    m_current_job = NULL;
}

void MainGui::sl_modifiedParameters(bool _changed)
{
    //qDebug() << "Receiving parameter value update flag : " << changed;

    bool has_actually_changed = _changed;

    if (!_changed) {
        qDebug() << "Canceled parameter value modification";
        has_actually_changed = m_parameters_widget->hasModifiedValues();

        if (has_actually_changed) {
            qDebug() << "But some parameter values remain modified";
        }
    }

    if (m_is_map_view) {
        m_job_parameter_modified = has_actually_changed;
        handleJobModified();

    } else {
        m_is_assembly_modified = has_actually_changed;
        handleAssemblyModified();
        if (has_actually_changed) {
            m_context.setLastActionPerformed(MODIFY_ASSEMBLY);
            enableActions();
        }
    }
}

void MainGui::sl_modifiedAssembly()
{
    //qDebug() << "Received assembly modified notification";

    m_is_assembly_modified = true;
    handleAssemblyModified();

    m_context.setLastActionPerformed(MODIFY_ASSEMBLY);
    enableActions();
}

void MainGui::sl_assemblyComplete(bool _is_complete)
{
    //qDebug() << "Received assembly completeness flag : " << isComplete;

    m_is_assembly_complete = _is_complete;
}

void MainGui::handleJobModified()
{
    if (!m_current_job) {
        qCritical() << "A job was modified but current job is not found";
        return;
    }

    QTreeWidgetItem * selected_item = m_ui->_TRW_assemblies->currentItem();
    if (!selected_item) {
        qCritical("Current job item is null (no selection)");
        return;
    }

    QString job_display_name = selected_item->text(0);

    bool marked_as_modified = job_display_name.right(1) == "*";

    /* mark job as modified */
    if (m_job_parameter_modified && !marked_as_modified) {
        selected_item->setText(0, job_display_name.append("*"));
    }

    /* restore job name */
    if (!m_job_parameter_modified && marked_as_modified) {
        selected_item->setText(0, m_current_job->name());
    }
}

void MainGui::handleAssemblyModified()
{
    if (m_new_assembly) {
        //qDebug() << "Updated new assembly " << _newAssembly->name();
        return;
    }

    if (!m_current_assembly) {
        qCritical() << "Notified for current assembly modification but current assembly is not found";
        return;
    }

    QTreeWidgetItem * selected_item = m_ui->_TRW_assemblies->currentItem();
    if (!selected_item) {
        qCritical("Current assembly item is null (no selection)");
        return;
    }

    QString assembly_display_name = selected_item->text(0);

    bool marked_as_modified = assembly_display_name.right(1) == "*";

    /* mark assembly as modified */
    if (m_is_assembly_modified && !marked_as_modified) {
        selected_item->setText(0, assembly_display_name.append("*"));
    }

    /* restore assembly name */
    if (!m_is_assembly_modified && marked_as_modified) {
        selected_item->setText(0, m_current_assembly->name());
    }
}

QString MainGui::getActualAssemblyOrJobName(QTreeWidgetItem* _current_item)
{
    QString assembly_or_job_name = _current_item->text(0);
    /* case job parameters were modified */
    /* case assembly assembly was modified (parameters or processing chain) */
    if (assembly_or_job_name.right(1) == "*") {
        assembly_or_job_name.chop(1); // remove trailing '*
    }

    return assembly_or_job_name;
}

QString MainGui::getActualNewAssemblyName()
{
    if (!m_new_assembly) {
        qCritical() << "No new assembly identified";
        return "";
    }

    QString assembly_name = m_new_assembly->name();
    /* new assembly should be suffixed by '*' */
    if (assembly_name.right(1) == "*") {
        assembly_name.chop(1); // remove trailing '*'
    } else {
        qWarning() << QString("New assembly name '%1' does not end with '*'").arg(assembly_name);
    }

    return assembly_name;
}

} // namespace matisse


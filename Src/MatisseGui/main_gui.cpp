#include <QStyle>
#include <QDesktopWidget>
#include <QMessageBox>

#include "main_gui.h"
#include "ui_main_gui.h"
#include "matisse_version_widget.h"
#include "visu_mode_widget.h"
#include "ongoing_process_widget.h"
#include "graphical_charter.h"
#include "remote_job_helper.h"

namespace matisse {

const QString MainGui::PREFERENCES_FILEPATH = QString("config/MatissePreferences.xml");
const QString MainGui::ASSEMBLY_EXPORT_PREFIX = QString("assembly_export_");
const QString MainGui::JOB_EXPORT_PREFIX = QString("job_export_");
const QString MainGui::JOB_REMOTE_PREFIX = QString("job_remote_");
const QString MainGui::DEFAULT_EXCHANGE_PATH = QString("exchange");
const QString MainGui::DEFAULT_ARCHIVE_PATH = QString("archive");
const QString MainGui::DEFAULT_REMOTE_PATH = QString("remote");
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
    m_export_path(""),
    m_import_path(""),
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
        m_preferences->setImportExportPath(DEFAULT_EXCHANGE_PATH);
        m_preferences->setArchivePath(DEFAULT_ARCHIVE_PATH);
        m_preferences->setDefaultResultPath(DEFAULT_RESULT_PATH);
        m_preferences->setDefaultMosaicFilenamePrefix(DEFAULT_MOSAIC_PREFIX);
        m_preferences->setProgrammingModeEnabled(false); // By default, programming mode is disabled
        m_preferences->setLanguage("FR");

        /* This case is obsolete : no need to initalize remote execution preferences */

        system_data_manager->writeMatissePreferences(PREFERENCES_FILEPATH, *m_preferences);
    } else {
        system_data_manager->readMatissePreferences(PREFERENCES_FILEPATH, *m_preferences);
    }

    updateLanguage(m_preferences->language(), true);

    updatePreferredDatasetParameters();
    m_archive_path = m_preferences->archivePath();
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

    // hide current process indicators
    resetOngoingProcessIndicators();

    initMainMenu();

    initContextMenus();

    initProcessWheelSignalling();

    initUserActions();

    initEngine();

    initVersionDisplay();

    initParametersWidget();

    initPreferences();

    initRemoteJobHelper();
    
    initAssemblyCreationScene();

    initMapFeatures();

    /* loading processors */
    initProcessorWidgets();

    /* swap to map view */
    sl_swapMapOrCreationView();

    /* notify widgets for initial color palette */
    emit si_updateColorPalette(m_current_color_set);

    // start current date/time timer
    initDateTimeDisplay();

    initWelcomeDialog();

    dpiScaleWidgets();

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
    m_colors_by_level.insert(ERROR, QString("level.red"));

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
    m_camera_manager_tool = new QAction(this);
    m_camera_calib_tool = new QAction(this);

    m_tool_menu->addAction(m_app_config_act);
    m_tool_menu->addSeparator();
    m_tool_menu->addAction(m_preprocessing_tool);
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
        m_data_viewer->load3DFile(info_result.absoluteFilePath(), _remove_previous_scenes);

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
    process_data_manager->loadArchivedJobs(m_archive_path);

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
    /* hide parameters view to display the dialog on top the dark background */
    bool previous_folding_state = m_ui->_PB_parameterFold->getIsUnfolded();
    doFoldUnfoldParameters(false);

    PreferencesDialog dialog(this, m_icon_factory, m_preferences, false);
    dialog.setFixedHeight(GraphicalCharter::instance().dpiScaled(PD_HEIGHT));
    dialog.setFixedWidth(GraphicalCharter::instance().dpiScaled(PD_WIDTH));
    dialog.setWindowFlags(Qt::Dialog | Qt::FramelessWindowHint);

    /* new preferences are saved if modified */
    if (dialog.exec() == QDialog::Accepted) {
        if (checkArchivePathChange()) {
            SystemDataManager::instance()->writeMatissePreferences(PREFERENCES_FILEPATH, *m_preferences);
            ProcessDataManager::instance()->loadArchivedJobs(m_preferences->archivePath());
            updateLanguage(m_preferences->language());
            updatePreferredDatasetParameters();
            // reset import / export path (will be reinitialized at runtime)
            m_export_path = "";
            m_import_path = "";

            /* recheck preferences for remote job execution */
            m_remote_job_helper->reinit();
        }
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

void MainGui::createExportDir()
{
    QString import_export_dir = m_preferences->importExportPath();
    if (import_export_dir.isEmpty()) {
        qDebug() << "Import export path not defined in preferences, using default path";
        import_export_dir = SystemDataManager::instance()->getDataRootDir() + QDir::separator() + DEFAULT_EXCHANGE_PATH;
    }

    m_export_path = import_export_dir + QDir::separator() + "export";

    QDir export_dir(m_export_path);
    if (!export_dir.exists()) {
        qDebug() << "Creating export directory " << m_export_path;
        export_dir.mkpath(".");
    }
}


void MainGui::createImportDir()
{
    QString import_export_dir = m_preferences->importExportPath();
    if (import_export_dir.isEmpty()) {
        qDebug() << "Import export path not defined in preferences, using default path";
        import_export_dir = SystemDataManager::instance()->getDataRootDir() + QDir::separator() + DEFAULT_EXCHANGE_PATH;
    }

    m_import_path = import_export_dir + QDir::separator() + "import";

    QDir importDir(m_import_path);
    if (!importDir.exists()) {
        qDebug() << "Creating import directory " << m_import_path;
        importDir.mkpath(".");
    }
}

void MainGui::checkArchiveDirCreated()
{
    QString archive_dir_path = m_preferences->archivePath();
    if (archive_dir_path.isEmpty()) {
        qDebug() << "Archive path not defined in preferences, using default path";
        archive_dir_path = SystemDataManager::instance()->getDataRootDir() + QDir::separator() + DEFAULT_ARCHIVE_PATH;
    }

    m_archive_path = archive_dir_path;

    QDir archive_dir(m_archive_path);
    if (!archive_dir.exists()) {
        qDebug() << "Creating archive directory " << m_archive_path;
        archive_dir.mkpath(".");
    }
}

void MainGui::checkRemoteDirCreated()
{
  bool already_checked = !m_remote_output_path.isEmpty();

  if (already_checked) {
    return;
  }

  m_remote_output_path = QStandardPaths::writableLocation(QStandardPaths::DataLocation) +
    QDir::separator() + "toServer"; 

  QDir remote_output_dir(m_remote_output_path);
  if (!remote_output_dir.exists()) {
      qDebug() << "Creating remote output directory " << m_remote_output_path;
      remote_output_dir.mkpath(".");
  }
}

void MainGui::updateJobStatus(
    QString _job_name, QTreeWidgetItem *_item, eMessageIndicatorLevel _indicator,
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
    checkArchiveDirCreated();

    if (!m_current_job) {
        // technically inconsistent
        qCritical() << QString("Current job not identified, could not archive");
        return;
    }

    bool archived = ProcessDataManager::instance()->archiveJobFiles(m_current_job->name(), m_archive_path);

    if (archived) {
        QMessageBox::information(this, tr("Backup"), tr("Task '%1' has been backed up")
                                 .arg(m_current_job->name()));

        // reload assembly tree
        sl_assembliesReload();

    } else {
        QMessageBox::critical(this, tr("Backup failure"), tr("Task '%1' couldn't be backed up.")
                              .arg(m_current_job->name()));
    }
}

void MainGui::sl_restoreJobs()
{
    if (!m_current_assembly) {
        qCritical() << "Could not identify selected assembly, impossible to restore jobs";
        return;
    }

    ProcessDataManager* process_data_manager = ProcessDataManager::instance();

    QString assembly_name = m_current_assembly->name();
    QStringList archived_jobs = process_data_manager->getAssemblyArchivedJobs(assembly_name);
    QStringList jobs_to_restore;

    RestoreJobsDialog dialog(this, assembly_name, archived_jobs, jobs_to_restore);
    dialog.setWindowFlags(Qt::Dialog | Qt::FramelessWindowHint);
    if (dialog.exec() != QDialog::Accepted) {
        return;
    }

    if (!jobs_to_restore.isEmpty()) {
        bool restored = process_data_manager->restoreArchivedJobs(m_archive_path, assembly_name, jobs_to_restore);
        if (restored) {
            sl_assembliesReload();
        }
    }

}

void MainGui::sl_duplicateJob()
{
    if (!m_current_job) {
        qCritical() << "The selected job cannot be identified, impossible to duplicate";
        return;
    }

    QString job_name = m_current_job->name();
    QString new_job_name;

    ProcessDataManager* process_data_manager = ProcessDataManager::instance();
    QStringList existing_job_names = process_data_manager->getJobsNames();
    QStringList archived_job_names = process_data_manager->getArchivedJobNames();

    DuplicateDialog dialog(this, job_name, new_job_name, false, existing_job_names, archived_job_names);
    dialog.setWindowFlags(Qt::Dialog | Qt::FramelessWindowHint);
    if (dialog.exec() != QDialog::Accepted) {
        return;
    }

    qDebug() << QString("Duplicating job %1 to %2").arg(job_name, new_job_name);
    bool duplicated = process_data_manager->duplicateJob(job_name, new_job_name);

    if (duplicated) {
        // reload assembly tree
        sl_assembliesReload();
        QMessageBox::information(this, tr("Task copy"), tr("Task '%1' copied")
                                 .arg(job_name));
    } else {
        QMessageBox::critical(this, tr("Copy failure"), tr("Task '%1' was not copied.")
                              .arg(job_name));
    }
}

void MainGui::sl_duplicateAssembly()
{
    if (!m_current_assembly) {
        qCritical() << "The selected assembly cannot be identified, impossible to duplicate";
        return;
    }

    QString assembly_name = m_current_assembly->name();
    QString new_assembly_name;

    ProcessDataManager* process_data_manager = ProcessDataManager::instance();

    QStringList existing_assembly_names = process_data_manager->getAssembliesList();

    DuplicateDialog dialog(this, assembly_name, new_assembly_name, true, existing_assembly_names);
    dialog.setWindowFlags(Qt::Dialog | Qt::FramelessWindowHint);
    if (dialog.exec() != QDialog::Accepted) {
        return;
    }

    qDebug() << QString("Duplicating assembly %1 to %2").arg(assembly_name, new_assembly_name);
    bool duplicated = process_data_manager->duplicateAssembly(assembly_name, new_assembly_name);

    if (duplicated) {
        // reload assembly tree
        sl_assembliesReload();
        QMessageBox::information(this, tr("Processing chain copy"), tr("Processing chain '%1' copied")
                                 .arg(assembly_name));
    } else {
        QMessageBox::critical(this, tr("Processing chain copy failed"), tr("Processing chain '%1' copy failed.")
                              .arg(assembly_name));
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
    if (m_export_path.isEmpty()) {
        createExportDir();
    }

    QString image_file_path = QFileDialog::getSaveFileName(this, tr("Export current view to image..."), m_export_path, tr("Image file (*.png)"));

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
        qCritical() << QString("Could not find exposure tool exe file '%1'").arg(tool_path);
        QMessageBox::critical(this, tr("Tool not found"), tr("Preprocessing tool not found in file '%1'").arg(tool_path));
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

    if (_is_for_remote_execution) {
        checkRemoteDirCreated();
    } else {
        if (m_export_path.isEmpty()) {
            createExportDir();
        }
    }

    /* common translated labels */
    QString export_failed_title = tr("Export failure");
    QString no_entity_selected_message = (_is_job_export_action) ? tr("No task selected.") : tr("No processing chain were selected.");
    QString export_title = (_is_job_export_action) ? tr("Taks export") : tr("Export processing chain");
    QString success_mssage = (_is_job_export_action) ? tr("Task '%1' exported in '%2'") : tr("Processing chain '%1' exported in '%2'");
    QString operation_failed_message = tr("Operation failed.");

    /* check that an assembly/job is selected */
    bool selected = (_is_job_export_action) ? (m_current_job != NULL) : (m_current_assembly != NULL);

    if (!selected) {
        qCritical() << "No assembly/job selected, impossible to export";
        QMessageBox::critical(this, export_failed_title, no_entity_selected_message);
        return;
    }


    QString entity_name = "";
    QString entity_file_name = "";
    QString entity_prefix = "";

    if (_is_job_export_action) {
        entity_name = m_current_job->name();
        entity_file_name = m_current_job->filename();
        entity_prefix = (_is_for_remote_execution) ? JOB_REMOTE_PREFIX : JOB_EXPORT_PREFIX;

    } else {
        entity_name = m_current_assembly->name();
        entity_file_name = m_current_assembly->filename();
        entity_prefix = ASSEMBLY_EXPORT_PREFIX;
    }

    qDebug() << QString("Exporting assembly/job '%1'...").arg(entity_name);

    QString export_path_root = (_is_for_remote_execution) ? m_remote_output_path : m_export_path;

    QString normalized_entity_name = entity_file_name;
    normalized_entity_name.chop(4); // remove ".xml" suffix
    QString export_filename = export_path_root + QDir::separator() + entity_prefix + normalized_entity_name + ".zip";

    QStringList file_names;

    QString assembly_name = "";
    AssemblyDefinition *assembly = NULL;

    ProcessDataManager* process_data_manager = ProcessDataManager::instance();

    if (_is_job_export_action) {
        QFileInfo job_file(process_data_manager->getJobsBasePath() + QDir::separator() + entity_file_name);
        if (!job_file.exists()) {
            qCritical() << QString("File not found for job '%1'. Job could not be exported.").arg(m_current_assembly->name());
            QMessageBox::critical(this, export_failed_title, tr("Task file '%1' does not exist.\n").arg(job_file.absoluteFilePath()).append(operation_failed_message));
            return;
        }

        file_names << job_file.absoluteFilePath();

        QFileInfo job_parameters_file(process_data_manager->getJobParametersFilePath(m_current_job->name()));
        if (!job_parameters_file.exists()) {
            qCritical() << QString("Parameters file not found for job '%1'. Job could not be exported.").arg(m_current_job->name());
            QMessageBox::critical(this, export_failed_title, tr("Task parameter file '%1' does not exist.\n").arg(job_parameters_file.absoluteFilePath())
                                  .append(operation_failed_message));
            return;
        }

        file_names << job_parameters_file.absoluteFilePath();

        assembly_name = m_current_job->assemblyName();

        assembly = process_data_manager->getAssembly(assembly_name);
        if (!assembly) {
            qCritical() << QString("Assembly '%1' could not be found in local repository").arg(assembly_name);
            QMessageBox::critical(this, export_failed_title, tr("Cannot load assembly '%1' parent from task '%2'.\n").arg(assembly_name).arg(entity_name)
                                  .append(operation_failed_message));
            return;
        }
    } else {
        assembly_name = entity_name;
        assembly = m_current_assembly;
    }

    QString assembly_filename = assembly->filename();

    QFileInfo assemblyFile(process_data_manager->getAssembliesPath() + QDir::separator() + assembly_filename);
    if (!assemblyFile.exists()) {
        qCritical() << QString("File not found for assembly '%1'. Assembly/Job could not be exported.").arg(assembly_name);
        QMessageBox::critical(this, export_failed_title, tr("Assembly file' '%1' does not exist.\n").arg(assemblyFile.absoluteFilePath())
                              .append(operation_failed_message));
        return;
    }

    file_names << assemblyFile.absoluteFilePath();

    QFileInfo assembly_parameters_file(process_data_manager->getAssembliesParametersPath() + QDir::separator() + assembly_filename);
    if (assembly_parameters_file.exists()) {
        file_names << assembly_parameters_file.absoluteFilePath();
    } else {
        qWarning() << QString("Parameters file not found for assembly '%1'. Assembly/Job will be exported without a parameters template file.").arg(assembly_name);
    }

    SystemDataManager* system_data_manager = SystemDataManager::instance();

    system_data_manager->writePlatformSummary();
    system_data_manager->writePlatformEnvDump();

    file_names << system_data_manager->getPlatformSummaryFilePath();
    file_names << system_data_manager->getPlatformEnvDumpFilePath();

    //FileUtils::zipFiles(exportFilename, ".", fileNames);
    FileUtils::zipFiles(export_filename, system_data_manager->getDataRootDir(), file_names);

    if (_is_for_remote_execution) {
        /* Store path for later use */
        m_current_remote_execution_bundle = export_filename;
    } else {
        /* Display export confirmation */
        QMessageBox::information(
                    this,
                    export_title,
                    success_mssage.arg(entity_name).arg(QDir(m_export_path).absolutePath()));
    }
}


void MainGui::executeImportWorkflow(bool _is_job_import_action) {
    if (m_import_path.isEmpty()) {
        createImportDir();
    }

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

    /* PROMPT USER FOR FILE TO IMPORT */
    QString file_dialog_title = (_is_job_import_action) ? tr("Import a task...") : tr("Import assembly...");
    QString import_file_path = QFileDialog::getOpenFileName(this, file_dialog_title, m_import_path, tr("Export file (*.zip)"));

    if (import_file_path.isEmpty()) {
        // cancelling import operation
        return;
    }

    /* CHECKING SELECTED IMPORT FILE */
    qDebug() << "Checking assembly/job import file...";

    SystemDataManager* system_data_manager = SystemDataManager::instance();
    ProcessDataManager* process_data_manager = ProcessDataManager::instance();

    QString data_root_dir = system_data_manager->getDataRootDir();
    QDir root(data_root_dir);

    QString user_data_path = system_data_manager->getUserDataPath();
    QString user_data_relative_path = root.relativeFilePath(user_data_path);

    QString assembly_file_pattern = process_data_manager->getAssemblyFilePattern();
    QString assembly_parameters_file_pattern = process_data_manager->getAssemblyParametersFilePattern();

    QRegExp assembly_rex(assembly_file_pattern);
    QRegExp assembly_parameters_rex(assembly_parameters_file_pattern);

    QString job_file_pattern = process_data_manager->getJobFilePattern();
    QString job_parameters_file_pattern = process_data_manager->getJobParametersFilePattern();

    QRegExp job_rex(job_file_pattern);
    QRegExp job_parameters_rex(job_parameters_file_pattern);

    QString remote_assembly_file_path = "";
    QString remote_assembly_parameters_file_path = "";

    QString remote_job_file = "";
    QString remote_job_parameters_file = "";

    QString platform_file_path = system_data_manager->getPlatformSummaryFilePath();
    QString remote_platform_file = root.relativeFilePath(platform_file_path);

    bool found_assembly_file = false;
    bool found_assembly_parameters_file = false;
    bool found_job_file = false;
    bool found_job_parameters_file = false;
    bool found_platform_file = false;

    QString import_failed_title = tr("Import failed");
    QString import_title = (_is_job_import_action) ? tr("Task import") : tr("Processing chain import");
    QString are_you_sure_continue_message = tr("Sure to continue ?");
    QString are_you_sure_replace_message = tr("Sure you want to erase ?");
    QString operation_failed_message = tr("Operation failed.");
    QString operation_temporarily_failed_message = tr("Operation temporary failed.\n");
    QString try_again_later_message = tr("Try again later.");
    QString remove_file_manually = tr("Delete manually or try again later.");

    // Parsing archive files to verify that all expected files are present
    QStringList files_list = FileUtils::getZipEntries(import_file_path);

    foreach (QString file, files_list) {
        if (file.startsWith(user_data_relative_path)) {
            if (assembly_rex.exactMatch(file)) {
                qDebug() << "Found assembly file " << file;
                remote_assembly_file_path = file;
                found_assembly_file = true;
            } else if (assembly_parameters_rex.exactMatch(file)) {
                qDebug() << "Found assembly parameters file " << file;
                remote_assembly_parameters_file_path = file;
                found_assembly_parameters_file = true;
            }

            if (_is_job_import_action) {
                if (job_rex.exactMatch(file)) {
                    qDebug() << "Found job file " << file;
                    remote_job_file = file;
                    found_job_file = true;
                } else if (job_parameters_rex.exactMatch(file)) {
                    qDebug() << "Found job parameters file " << file;
                    remote_job_parameters_file = file;
                    found_job_parameters_file = true;
                }
            }
        } else {
            if (file == remote_platform_file) {
                qDebug() << "Found platform file " << remote_platform_file;
                found_platform_file = true;
            }
        }
    }

    if (_is_job_import_action) {
        if (!found_job_file) {
            qCritical() << "Job file could not be found in the export archive " + import_file_path;
            QMessageBox::critical(this, import_failed_title, tr("Job file could not be found in the export archive."));
            return;
        }

        if (!found_job_parameters_file) {
            qCritical() << "Job parameters file could not be found in the export archive " + import_file_path;
            QMessageBox::critical(this, import_failed_title, tr("Job parameters file could not be found in the export archive."));
            return;
        }
    }

    if (!found_assembly_file) {
        qCritical() << "Assembly file could not be found in the export archive " + import_file_path;
        QMessageBox::critical(this, import_failed_title, tr("Assembly file could not be found in the export archive."));
        return;
    }

    if (!found_assembly_parameters_file) {
        qCritical() << "Assembly parameters file could not be found in the export archive " + import_file_path;
        QMessageBox::critical(this, import_failed_title, tr("Assembly parameters file could not be found in the export archive ."));
        return;
    }

    if (!found_platform_file) {
        qCritical() << "Remote platform summary file could not be found in the export archive " + import_file_path;
        QMessageBox::critical(this, import_failed_title, tr("Remote platform summary file could not be found in the export archive."));
        return;
    }

    /* EXTRACTING ASSEMBLY EXPORT FILE */
    qDebug() << QString("Extracting assembly/job export file '%1'").arg(import_file_path);

    QString temp_import_dir_path;
    bool temp_dir_created = FileUtils::createTempDirectory(temp_import_dir_path);

    if (!temp_dir_created) {
        qCritical() << QString("Could not create temporary importation dir '%1'.").arg(temp_import_dir_path);
        QMessageBox::critical(this, import_failed_title, operation_temporarily_failed_message.append(try_again_later_message));
        return;
    }

    bool unzipped = FileUtils::unzipFiles(import_file_path, temp_import_dir_path);

    if (!unzipped) {
        qCritical() << QString("Could not unzip file '%1' to temporary importation dir '%2'.").arg(import_file_path).arg(temp_import_dir_path);
        QMessageBox::critical(this, import_failed_title, tr("Could not unzip file '%1'.\n")
                              .arg(import_file_path).append(try_again_later_message));
        return;
    }

    /* COMPARING REMOTE AND LOCAL PLATFORM */
    QString temp_remote_platform_file = temp_import_dir_path + QDir::separator() + remote_platform_file;

    bool read = system_data_manager->readRemotePlatformSummary(temp_remote_platform_file);
    if (!read) {
        QMessageBox::critical(this, import_failed_title, tr("Import file invalid.\n").append(operation_failed_message));
        return;
    }

    PlatformComparisonStatus *comparison_status = system_data_manager->compareRemoteAndLocalPlatform();

    if (!comparison_status) {
        QMessageBox::critical(this, import_failed_title, tr("No information available on distant platform.\n").append(operation_failed_message));
        return;
    }


    if (comparison_status->doesMatch()) {
        qDebug() << "Remote and local platforms are identical.";
    } else {

        /* Comparing remote and local Matisse versions */

        if (comparison_status->matisseCompared().m_version_compare == DIFFERENT) {
            QMessageBox::critical(this, import_failed_title, tr("Distant and local Matisse version couldn't be compared.\n").append(operation_failed_message));
            return;
        } else if (comparison_status->matisseCompared().m_version_compare == NEWER) {
            QMessageBox::critical(this, import_failed_title, tr("Archive comes from newer Matisse version than this one.\n").append(operation_failed_message));
            return;
        } else if (comparison_status->matisseCompared().m_version_compare == OLDER) {
            if (QMessageBox::No == QMessageBox::question(
                        this,
                        import_title,
                        tr("You are trying to import from an older version of Matisse. Sure to continue ?\n").append(are_you_sure_continue_message),
                        QMessageBox::Yes,
                        QMessageBox::No)) {
                return;
            }
        }
        /* If Matisse versions are identical, execution continues */

        QStringList platform_gap_messages = comparison_status->getPlatformGapMessages();
        if (!platform_gap_messages.isEmpty()) {

            // build stack for platform gap messages
            QString platform_gap_messages_stack;

            for (int i = 0; i < platform_gap_messages.size() ; i++) {
                QString message = platform_gap_messages.at(i);
                platform_gap_messages_stack.append(i).append(": ").append(message);
                if (i < platform_gap_messages.size() - 1) {
                    platform_gap_messages_stack.append("\n");
                }
            }

            if (QMessageBox::No == QMessageBox::question(
                        this,
                        import_title,
                        tr("You are trying to import a file from a different system version (see below).\n").append(are_you_sure_continue_message)
                        .append("\n").append(platform_gap_messages_stack),
                        QMessageBox::Yes,
                        QMessageBox::No)) {
                return;
            }
        }

    }

    /* CHECKING JOB / ASSEMBLY CONSISTENCY */

    QString source_job_file_path = "";
    QString job_name = "";
    QString assembly_name;

    if (_is_job_import_action) {
        if (!m_current_assembly) {
            /* Impossible case : assembly has to be selected to provide access to job import function,
             * if this happens, the method is called by wrong caller */
            qCritical() << "No assembly selected, impossible to import job";
            return;
        }

        qDebug() << "Checking if imported job is consistent with selected assembly...";

        QString temp_user_data_path = temp_import_dir_path + "/" + user_data_relative_path;

        // instantiate new data manager, not forgetting to delete it after use
        ProcessDataManager* temp_data_mgr = ProcessDataManager::newTempInstance();
        temp_data_mgr->init(temp_import_dir_path, temp_user_data_path);

        source_job_file_path = temp_import_dir_path + "/" + remote_job_file;
        QFileInfo temp_job_file_info(source_job_file_path);
        if (!temp_job_file_info.exists()) {
            // Technically unconsistent case : something went wront when unzipping
            qCritical() << "Temp job file not found, archive unzipping failed";
            ProcessDataManager::deleteTempInstance();
            return;
        }

        QString temp_job_file_name = temp_job_file_info.fileName();
        bool read = temp_data_mgr->readJobFile(temp_job_file_name);

        if (!read) {
            qCritical() << QString("The job file contained in export archive is invalid.");
            QMessageBox::critical(this, import_failed_title, tr("Task file in archive is invalid.\n")
                                  .append(operation_failed_message));
            ProcessDataManager::deleteTempInstance();
            return;
        }

        QStringList temp_job_names = temp_data_mgr->getJobsNames();
        if (temp_job_names.isEmpty()) {
            // technically unconsistent case
            qCritical() << "Source job file was read from export archive but was not handled correctly by the data manager";
            ProcessDataManager::deleteTempInstance();
            return;
        }

        job_name = temp_job_names.at(0);
        JobDefinition *job = temp_data_mgr->getJob(job_name);

        ProcessDataManager::deleteTempInstance();

        if (!job) {
            // technically unconsistent case
            qCritical() << "Source job file was read from export archive but was not handled correctly by the data manager";
            return;
        }

        assembly_name = job->assemblyName();

        if (assembly_name != m_current_assembly->name()) {
            qCritical() << QString("The job file contained in export archive is invalid.").arg(source_job_file_path);
            QMessageBox::critical(this, import_failed_title, tr("The task you're trying to import does not match the selected assembly.\n")
                                  .append(operation_failed_message));
            return;
        }
    }


    /* CHECKING IF JOB/ASSEMBLY FILES ARE ALREADY PRESENT ON LOCAL PLATFORM */

    QString target_job_file_path = "";
    QString target_job_parameters_file_path = "";

    QString target_assembly_file_path = data_root_dir + "/" + remote_assembly_file_path;
    QFile target_assembly_file(target_assembly_file_path);

    QString target_assembly_parameters_file_path = data_root_dir + "/" + remote_assembly_parameters_file_path;
    QFile target_assembly_parameters_file(target_assembly_parameters_file_path);

    QString source_assembly_file_path = temp_import_dir_path + "/" + remote_assembly_file_path;
    QFile source_assembly_file(source_assembly_file_path);

    QString source_assembly_parameters_file_path = temp_import_dir_path + "/" + remote_assembly_parameters_file_path;
    QFile source_assembly_parameters_file(source_assembly_parameters_file_path);

    bool remove_target_job_file = false;
    bool remove_target_job_parameters_file = false;
    bool remove_target_assembly_file = false;
    bool remove_target_assembly_parameters_file = false;
    bool import_assembly_files = false;

    QFile target_job_file;
    QFile target_job_parameters_file;

    qDebug() << "Checking if job/assembly are already present on local platform...";

    if (_is_job_import_action) {
        target_job_file_path = data_root_dir + "/" + remote_job_file;
        target_job_file.setFileName(target_job_file_path);
        target_job_parameters_file_path = data_root_dir + "/" + remote_job_parameters_file;
        target_job_parameters_file.setFileName(target_job_parameters_file_path);

        if (target_job_file.exists()) {
            qDebug() << "Job already present on local platform";
            if (QMessageBox::No == QMessageBox::question(
                        this,
                        import_title,
                        tr("Task %1 already exists.\n")
                        .arg(job_name).append(are_you_sure_replace_message),
                        QMessageBox::Yes,
                        QMessageBox::No)) {
                return;
            }

            remove_target_job_file = true;

            if (target_job_parameters_file.exists()) {
                qDebug() << "Job parameters file already present on local platform";
                remove_target_job_parameters_file = true;
            } else {
                /* Technically unconsistent case : the job exists but no job parameters file was found */
                qWarning() << QString("The job '%1' exists on local platform but the job parameters file was not found").arg(job_name);
            }

        } else { // Job file does not exist on local platform

            if (target_job_parameters_file.exists()) {
                /* Technically unconsistent case : the job does not exist on local platform but a job parameters file was found with a corresponding name */
                qWarning() << QString("The job parameters file for job '%1' was found on local platform but job definition file does not exist").arg(job_name);
                remove_target_job_parameters_file = true;
            }
        }

        if (!target_assembly_file.exists()) {
            /* Technically unconsistent case : the assembly was selected but its file does not exist */
            qCritical() << QString("The assembly definition file for assembly '%1' was not found on local platform, job import failed").arg(m_current_assembly->name());
            return;
        }

        // check that assembly and assembly parameters file are identical
        if (FileUtils::areFilesIdentical(source_assembly_file_path, target_assembly_file_path)) {
            qDebug() << "Remote and local assembly definition files are identical";
        } else {
            qDebug() << "Remote and local assembly definition files are different";
            if (QMessageBox::No == QMessageBox::question(
                        this,
                        import_title,
                        tr("Processing chain %1 in archive is different from the one on your computer.\n")
                        .arg(m_current_assembly->name()).append(are_you_sure_replace_message),
                        QMessageBox::Yes,
                        QMessageBox::No)) {
                return;
            }
        }

        remove_target_assembly_file = true;
        import_assembly_files = true;

        if (target_assembly_parameters_file.exists()) {
            if (FileUtils::areFilesIdentical(source_assembly_parameters_file_path, target_assembly_parameters_file_path)) {
                qDebug() << "Remote and local assembly parameters files are identical";
            } else {
                qDebug() << "Remote and local assembly parameters files are different";
                if (QMessageBox::No == QMessageBox::question(
                            this,
                            import_title,
                            tr("Parameters from processing chain %1 in archive is different from the one on your computer.\n")
                            .arg(m_current_assembly->name()).append(are_you_sure_replace_message),
                            QMessageBox::Yes,
                            QMessageBox::No)) {
                    return;
                }
            }

            remove_target_assembly_parameters_file = true;

        } else {
            /* Technically unconsistent case : the assembly exists but no assembly parameters file was found */
            qWarning() << QString("The assembly parameters file for assembly '%1' was found on local platform but assembly definition file does not exist").arg(m_current_assembly->name());
            if (QMessageBox::No == QMessageBox::question(
                        this,
                        import_title,
                        tr("Processing chain %1 exists on this computer but has no parameters file.\n")
                        .arg(m_current_assembly->name()).append(are_you_sure_replace_message),
                        QMessageBox::Yes,
                        QMessageBox::No)) {
                return;
            }
        }

    } else { // Assembly import action

        import_assembly_files = true;

        if (target_assembly_file.exists()) {
            qDebug() << "Assembly already present on local platform";
            assembly_name = process_data_manager->getAssemblyNameByFilePath(target_assembly_file_path);

            if (QMessageBox::No == QMessageBox::question(
                        this,
                        import_title,
                        tr("Processing chain %1 already exists on this computer.\n")
                        .arg(assembly_name).append(are_you_sure_replace_message),
                        QMessageBox::Yes,
                        QMessageBox::No)) {
                return;
            }

            remove_target_assembly_file = true;
        }

        if (target_assembly_parameters_file.exists()) {
            if (target_assembly_file.exists()) {
                qDebug() << "Assembly parameters file already present on local platform";
            } else {
                /* Technically unconsistent case */
                qWarning() << QString("The assembly parameters file for assembly '%1' was found on local platform but assembly definition file does not exist").arg(assembly_name);
            }

            remove_target_assembly_parameters_file = true;
        }

    } // end if (isJobImportAction)


    /* REMOVING FILES TO BE REPLACED */
    qDebug() << "Removing existing assembly/job files...";

    if (remove_target_job_file) {
        if (!target_job_file.remove()) {
            qCritical() << QString("Could not remove existing job file '%1'.").arg(target_job_file_path);
            QMessageBox::critical(this, import_failed_title, tr("Could not remove existing job file '%1'.\n")
                                  .arg(target_job_file_path).append(remove_file_manually));
            return;
        }
    }

    if (remove_target_job_parameters_file) {
        if (!target_job_parameters_file.remove()) {
            qCritical() << QString("Could not remove existing job parameters file '%1'.").arg(target_job_parameters_file_path);
            QMessageBox::critical(this, import_failed_title, tr("Could not remove existing job parameters file '%1'.\n")
                                  .arg(target_job_parameters_file_path).append(remove_file_manually));
            return;
        }
    }

    if (remove_target_assembly_file) {
        if (!target_assembly_file.remove()) {
            qCritical() << QString("Could not remove existing assembly file '%1'.").arg(target_assembly_file_path);
            QMessageBox::critical(this, import_failed_title, tr("Could not remove existing assembly file '%1'.\n")
                                  .arg(target_assembly_file_path).append(remove_file_manually));
            return;
        }
    }

    if (remove_target_assembly_parameters_file) {
        if (!target_assembly_parameters_file.remove()) {
            qCritical() << QString("Could not remove existing assembly parameters file '%1'.").arg(target_assembly_parameters_file_path);
            QMessageBox::critical(this, import_failed_title, tr("Could not remove existing assembly parameters file '%1'.\n")
                                  .arg(target_assembly_parameters_file_path).append(remove_file_manually));
            return;
        }
    }


    /* PROCEEDING WITH IMPORT */
    qDebug() << "Proceeding with assembly/job import...";
    if (_is_job_import_action) {
        QFile source_job_file(source_job_file_path);

        qDebug() << "Copying job file...";
        if (!source_job_file.copy(target_job_file_path)) {
            qCritical() << QString("Could not copy job file '%1' to '%2'.").arg(source_job_file_path).arg(target_job_file_path);
            QMessageBox::critical(this, import_failed_title, tr("Could not copy job file"));
            return;
        } else {
            qDebug() << "... done";
        }

        QString source_job_parameters_file_path = temp_import_dir_path + "/" + remote_job_parameters_file;
        QFile source_job_parameters_file(source_job_parameters_file_path);

        qDebug() << "Copying job parameters file...";
        if (!source_job_parameters_file.copy(target_job_parameters_file_path)) {
            qCritical() << QString("Could not copy job parameters file '%1' to '%2'.").arg(source_job_file_path).arg(target_job_file_path);
            QMessageBox::critical(this, import_failed_title, tr("Could not copy job parameters file"));
            return;
        } else {
            qDebug() << "... done";
        }
    }

    // Assembly files are copied in case of assembly import, or job import with user confirmation for replacement
    if (import_assembly_files) {
        qDebug() << "Copying assembly file...";
        if (!source_assembly_file.copy(target_assembly_file_path)) {
            qCritical() << QString("Could not copy assembly file '%1' to '%2'.").arg(source_assembly_file_path).arg(target_assembly_file_path);
            QMessageBox::critical(this, import_failed_title, tr("Could not copy assembly file"));
            return;
        } else {
            qDebug() << "... done";
        }

        qDebug() << "Copying assembly parameters file...";
        if (!source_assembly_parameters_file.copy(target_assembly_parameters_file_path)) {
            qCritical() << QString("Could not copy assembly parameters file '%1' to '%2'.").arg(source_assembly_file_path).arg(target_assembly_file_path);
            QMessageBox::critical(this, import_failed_title, tr("Could not copy assembly parameters file"));
            return;
        } else {
            qDebug() << "... done";
        }
    }

    /* Reload assembly tree */
    sl_assembliesReload();

    /* Displaying success message */
    QString success_message = "";

    if (_is_job_import_action) {
        if (job_name.isEmpty()) {
            job_name = process_data_manager->getJobNameByFilePath(source_job_file_path);
        }

        success_message = tr("Task '%1' successfully imported.").arg(job_name);
    } else {
        if (assembly_name.isEmpty()) {
            assembly_name = process_data_manager->getAssemblyNameByFilePath(source_assembly_file_path);
        }

        success_message = tr("Processing chain '%1' successfully imported.").arg(assembly_name);
    }

    QMessageBox::information(
                this,
                import_title,
                success_message);
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


bool MainGui::checkArchivePathChange()
{
    if (!m_archive_path.isEmpty()) {
        QString new_archive_path = m_preferences->archivePath();
        if (new_archive_path != m_archive_path) {
            qDebug() << "Archive path has changed, checking if jobs were already archived...";
            bool has_archived_files = false;

            QDir archive_dir(m_archive_path);
            if (archive_dir.exists()) {
                QStringList entries = archive_dir.entryList(QDir::Files);
                if (!entries.isEmpty()) {
                    foreach (QString entry, entries) {
                        if (entry.endsWith(".xml")) {
                            has_archived_files = true;
                            break;
                        }
                    }
                }
            }

            if (has_archived_files) {
                QString full_archive_path = archive_dir.absolutePath();
                QMessageBox::critical(this, tr("Archive folder"), tr("Archive folder '%1' already contains task(s) and cannot be modified.")
                                      .arg(full_archive_path));
                /* restoring previous archive path */
                m_preferences->setArchivePath(m_archive_path);
                return false;
            }
        }
    }

    return true;
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
            if (checkArchivePathChange()) {
                SystemDataManager::instance()->writeMatissePreferences(PREFERENCES_FILEPATH, *m_preferences);
                ProcessDataManager::instance()->loadArchivedJobs(m_preferences->archivePath());
                updateLanguage(m_preferences->language());
                updatePreferredDatasetParameters();
                // reset import / export path (will be reinitialized at runtime)
                m_export_path = "";
                m_import_path = "";
            }
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
    m_data_viewer->load3DFile(_filepath_p);
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

        qDebug() << "Saving assembly " << selected_item;

        AssemblyDefinition *assembly = ProcessDataManager::instance()->getAssembly(assembly_name);
        if (assembly) {
            // mise à jour des propriétés
            KeyValueList *props = m_assemblies_properties.value(assembly_name);

            AssemblyDialog dialog(this, assembly_name, *props, false, false);
            dialog.setWindowFlags(Qt::Dialog | Qt::FramelessWindowHint);
            if (dialog.exec() != QDialog::Accepted) {
                return;
            }

            assembly->setVersion(props->getValue("Version"));
            assembly->setUsable(props->getValue("Valid").toInt());
            assembly->setComment(props->getValue("Comments"));

            saveAssemblyAndReload(assembly);

            m_context.setLastActionPerformed(SAVE_ASSEMBLY);
            enableActions();
        } else {
            qWarning() << "Assembly not found in session repository : " << assembly_name;

            // on désactive la vue graphique
            m_assembly_editor->getScene()->reset();
            m_assembly_editor->getGraphicsView()->setEnabled(false);
        }

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
    // reinitialisation des parametres avec leurs valeurs par defaut
    m_engine.parametersManager()->restoreParametersDefaultValues();

    // chargement des parametres attendus par les modules de l'assemblage
    m_engine.parametersManager()->clearExpectedParameters();
    m_engine.addParametersForInputDataProvider(_selected_assembly->sourceDefinition()->name());
    foreach (ProcessorDefinition* processor, _selected_assembly->processorDefs()) {
        m_engine.addParametersForProcessor(processor->name());
    }
    m_engine.addParametersForOutputDataWriter(_selected_assembly->destinationDefinition()->name());

    // chargement des valeurs de paramètres du template de l'assemblage
    m_engine.parametersManager()->loadParameters(_selected_assembly->name(), true);
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

// TODO Verif modif chrisar & ifremer simultanee (bug reload)
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
        // rien de sélectionné...
        qCritical() << "No item was selected";
        return;
    }

    if (selected_item->childCount() > 0) {
        // jobs exist... removal prohibited
        // this cas should not occur : controlled at contextual menu creation
        qCritical() << "Jobs are attached to the assembly, it cannot be removed.";
        return;
    }

    QString assembly_name = getActualAssemblyOrJobName(selected_item);

    ProcessDataManager* process_data_manager = ProcessDataManager::instance();

    if (process_data_manager->assemblyHasArchivedJob(assembly_name)) {
        QMessageBox::critical(this, tr("Invalid path"), tr("Processing chain has archived task and cannot be removed.").arg(assembly_name));
        return;
    }

    if (_prompt_user) {
        int ret = QMessageBox::question(this, tr("Delete assembly"), tr("Do you want to delete assembly %1 ?").arg(assembly_name), QMessageBox::Ok | QMessageBox::Cancel, QMessageBox::Cancel);
        if (ret==QMessageBox::Cancel) {
            return;
        }
    }

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

    AssemblyDefinition *assembly = process_data_manager->getAssembly(assembly_name);
    if (!assembly) {
        qCritical() << QString("Assembly '%1' selected to be removed was not found in local repository").arg(assembly_name);
        return;
    }

    qDebug() << "Removing assembly and assembly parameters files...";

    QString filename = process_data_manager->getAssembly(assembly_name)->filename();
    // suppression fichier assemblage
    QString assembly_filepath = process_data_manager->getAssembliesPath() + QDir::separator() + filename;
    QString param_filepath = process_data_manager->getAssembliesParametersPath() + QDir::separator() + filename;

    QFile file(assembly_filepath);
    if (!file.exists()) {
        qCritical() << QString("Assembly file '%1' does not exist, cannot be removed").arg(assembly_filepath);
        return;
    }

    if (!file.remove()) {
        qCritical() << QString("Error removing assembly file '%1'").arg(assembly_filepath);
        return;
    }

    // La suppression du fichier de parametre est controllee mais pas bloquante en cas d'echec
    QFile param_file(param_filepath);
    if (param_file.exists()) {
        if (param_file.remove()) {
            qDebug() << "... done";
        } else {
            qCritical() << QString("Error removing assembly parameters file '%1'").arg(param_filepath);
        }
    } else {
        qCritical() << QString("Assembly parameters file '%1' does not exist, cannot be removed").arg(param_filepath);
    }

    // suppression liste: rechargement...
    sl_assembliesReload();
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

    ProcessDataManager* process_data_manager = ProcessDataManager::instance();

    AssemblyDefinition * assembly = process_data_manager->getAssembly(assembly_name);

    if (!assembly) {
        qCritical() << QString("Assembly '%1' not found in repository").arg(assembly_name);
        return;
    }

    qDebug() << "Creating new job";
    KeyValueList kvl;
    kvl.append("name");
    kvl.append("comment");

    kvl.append(DATASET_PARAM_DATASET_DIR);
    kvl.append(DATASET_PARAM_NAVIGATION_FILE);

    /* Key value list is initialized with default dataset preferences */
    kvl.set(DATASET_PARAM_OUTPUT_DIR, m_preferences->defaultResultPath());
    kvl.set(DATASET_PARAM_OUTPUT_FILENAME, m_preferences->defaultMosaicFilenamePrefix());

    /* Dataset parameters are loaded / overriden from template assembly parameters */
    m_engine.parametersManager()->pullDatasetParameters(kvl);

    QString jobs_base_path = process_data_manager->getJobsBasePath();
    QStringList job_names = process_data_manager->getJobsNames();
    QStringList archived_job_names = process_data_manager->getArchivedJobNames();

    JobDialog dialog(this, m_icon_factory, &kvl, jobs_base_path, job_names, archived_job_names);
    dialog.setFixedHeight(GraphicalCharter::instance().dpiScaled(JD_HEIGHT));
    dialog.setFixedWidth(GraphicalCharter::instance().dpiScaled(JD_WIDTH));
    dialog.setWindowFlags(Qt::Dialog | Qt::FramelessWindowHint);
    if (dialog.exec() != QDialog::Accepted) {
        return;
    }

    QString assembly_version = assembly->version().simplified();

    QString job_name = kvl.getValue("name");

    JobDefinition new_job(job_name, assembly_name, assembly_version);
    new_job.setComment(kvl.getValue("comment"));

    ExecutionDefinition execution_definition;
    execution_definition.setExecuted(false);
    new_job.setExecutionDefinition(&execution_definition);

    RemoteJobDefinition remote_job_definition;
    remote_job_definition.setScheduled(false);
    new_job.setRemoteJobDefinition(&remote_job_definition);

    if (!process_data_manager->writeJobFile(&new_job)) {
        qCritical() << QString("Job definition file could not be created for new job '%1'").arg(job_name);
        return;
    }

    m_engine.parametersManager()->createJobParametersFile(assembly_name, job_name, kvl);

    // load job definition to local repository
    QString job_filename = job_name;
    job_filename.append(".xml");
    process_data_manager->readJobFile(job_filename);
    JobDefinition* new_job_def = process_data_manager->getJob(job_name);

    // display and select new job (flag is set to false because job was already saved)
    if (addJobInTree(new_job_def, false)) {
        selectJob(job_name);
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

    ProcessDataManager* process_data_manager = ProcessDataManager::instance();


    if (!m_current_job) {
        qWarning() << "Current job not identified";

        QString job_name = getActualAssemblyOrJobName(item);

        m_current_job = process_data_manager->getJob(job_name);

        if (!m_current_job) {
            qCritical() << QString("Selected job '%1' not found in local repository").arg(job_name);
            return;
        }
    }

    // modif logique: on peut enregistrer un job deja execute: on enleve l'etat executed et on ecrase...
    if (m_current_job->executionDefinition()->executed()) {
        if (QMessageBox::No == QMessageBox::question(this, tr("Processed job..."),
                                                     tr("Job already processed.\n Do you still want to reprocess ?"),
                                                     QMessageBox::Yes,
                                                     QMessageBox::No)) {
            return;
        }
        m_current_job->executionDefinition()->setExecuted(false);
        m_current_job->executionDefinition()->setExecutionDate(QDateTime());
        m_current_job->executionDefinition()->setResultFileNames(QStringList() << "");
        // changement etat led + effacement image

        if (item->type() == MatisseTreeItem::Type) {
            MatisseTreeItem *job_item = static_cast<MatisseTreeItem *>(item);
            IconizedWidgetWrapper *item_wrapper = new IconizedTreeItemWrapper(job_item, 0);
            m_icon_factory->attachIcon(item_wrapper, "lnf/icons/led.svg", false, false, m_colors_by_level.value(IDLE));
        } else {
            qCritical() << QString("Item for job '%1' is not of type MatisseTreeItem, cannot display icon").arg(m_current_job->name());
        }

        //        item->setIcon(0, *greyLedIcon);
        m_data_viewer->clear();
    }


    /* save job and parameters */
    if (!process_data_manager->writeJobFile(m_current_job, true)) {
        showError(tr("Job file..."), tr("File %1 could not be write...").arg(m_current_job->name() + ".xml"));
        return;
    }

    m_engine.parametersManager()->saveParametersValues(m_current_job->name(), false);

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
        // rien de sélectionné...
        return;
    }
    if (selected_item->parent() == NULL) {
        // assemblage... impossible de supprimer
        // traité ailleurs normalement...
        return;
    }

    QString job_name = getActualAssemblyOrJobName(selected_item);

    int ret = QMessageBox::question(this, tr("Delete Job"), tr("Do you want to delete Job %1?").arg(job_name), QMessageBox::Ok | QMessageBox::Cancel, QMessageBox::Cancel);
    if (ret==QMessageBox::Cancel) {
        return;
    }

    ProcessDataManager* process_data_manager = ProcessDataManager::instance();

    QString filename = process_data_manager->getJob(job_name)->filename();
    // suppression fichier job

    filename = process_data_manager->getJobsBasePath() + QDir::separator() + filename;
    QFile file(filename);
    if (!file.exists()) {
        qCritical() << QString("Job file '%1' not found, impossible to remove").arg(filename);
        return;
    }

    if (!file.remove()) {
        qCritical() << QString("Job file '%1' could not be removed").arg(filename);
        return;
    }

    filename = process_data_manager->getJobParametersFilePath(job_name);

    QFile parameter_file(filename);

    if (!parameter_file.exists()) {
        qCritical() << QString("Job parameters file '%1' not found, impossible to remove").arg(filename);
    } else {
        if (!parameter_file.remove()) {
            qCritical() << QString("Job parameters file '%1' could not be removed").arg(filename);
        }
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

void MainGui::showError(QString _title, QString _message) {
    qCritical() << _title << " - " << _message;
    QMessageBox::warning(this, _title, _message);
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

void MainGui::showStatusMessage(QString _message, eMessageIndicatorLevel _level)
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
    // fold parameters widget so the dialog is displayed on top of the dark background
    doFoldUnfoldParameters(false);

    sl_clearAssembly();
    m_engine.parametersManager()->restoreParametersDefaultValues();

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
    QString display_name = name;
    display_name.append('*');

    m_new_assembly = new AssemblyDefinition();
    m_new_assembly->setName(display_name);
    m_new_assembly->setCreationDate(QDate::fromString(fields.getValue("Date")));
    m_new_assembly->setDate(fields.getValue("Date"));
    m_new_assembly->setVersion(fields.getValue("Version"));
    m_new_assembly->setAuthor(fields.getValue("Author"));
    m_new_assembly->setUsable(fields.getValue("Valid").toInt());
    m_new_assembly->setComment(fields.getValue("Comments"));

    // filename is derived from the assembly name
    QString filename = ProcessDataManager::instance()->fromNameToFileName(name);
    m_new_assembly->setFilename(filename);

    QTreeWidgetItem *item = addAssemblyInTree(m_new_assembly);

    m_ui->_TRW_assemblies->setCurrentItem(item);
    item->setSelected(true);

    m_current_assembly = NULL;

    displayAssemblyProperties(m_new_assembly);

    // unfold parameters
    doFoldUnfoldParameters(true);

    m_context.setLastActionPerformed(CREATE_ASSEMBLY);
    enableActions();

    // activer vue graphique
    m_assembly_editor->getGraphicsView()->setEnabled(true);

}

void MainGui::enableActions()
{
    eUserAction last_action = m_context.lastActionPerformed();

    /* enable/diable main menu items */
    m_export_map_view_act->setEnabled(m_is_map_view);
    m_map_toolbar_act->setEnabled(m_is_map_view);
    m_preprocessing_tool->setEnabled(m_is_map_view);
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

    // clear log
    m_ui->_QTE_loggingText->clear();

    // On teste assemblage ou job
    // Le bouton est actif si le job est selectionnable
    // cad si l'image n'a pas été produite...
    // ou si on a selectionné un assemblage
    // dans ce cas, on sauvegarde le job avant de le lancer...

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

    ProcessDataManager* process_data_manager = ProcessDataManager::instance();

    if (!m_current_job) {
        qWarning() << "Current job not identified";

        QString job_name = getActualAssemblyOrJobName(current_item);

        m_current_job = process_data_manager->getJob(job_name);

        if (!m_current_job) {
            qCritical() << QString("Selected job '%1' not found in local repository, impossible to launch").arg(job_name);
            return;
        }
    } else {
        job_name = m_current_job->name();
    }

    QString assembly_name = m_current_job->assemblyName();

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

    m_engine.parametersManager()->saveParametersValues(job_name, false);

    AssemblyDefinition * assembly_def = process_data_manager->getAssembly(assembly_name);
    if (!assembly_def) {
        qCritical() << "Assembly error" << assembly_name;
        showStatusMessage(tr("Assembly error"), ERROR);
        return;
    }

    QString job_filename = m_current_job->filename();

    qDebug() << "Running job " << job_name;

    QString msg1=tr("File not found.");
    QString msg2=tr("Job file %1 could not be launched").arg(job_name);

    // on recharge le fichier...
    if (!process_data_manager->readJobFile(job_filename)) {
        qCritical() << QString("Error reading job file for job '%1' before launch").arg(job_name);
        QMessageBox::information(this, msg1, msg2);
        showStatusMessage(msg1+ " " + msg2, ERROR);
        return;
    }

    m_current_job = process_data_manager->getJob(job_name);

    if (!m_current_job) {
        qCritical() << QString("Job '%1' could not be loaded properly before launch").arg(job_name);
        QMessageBox::information(this, msg1, msg2);
        showStatusMessage(msg1+ " " + msg2, ERROR);
        return;
    }

    /* Copy XML files to result path */
    QString result_path = m_engine.parametersManager()->getParameterValue(DATASET_PARAM_OUTPUT_DIR);
    QDir result_path_dir(result_path);
    if (result_path_dir.isRelative())
    {
        QString data_path = m_engine.parametersManager()->getParameterValue(DATASET_PARAM_DATASET_DIR);
        result_path = data_path + QDir::separator() + result_path_dir.path();
    }

    process_data_manager->copyJobFilesToResult(job_name, result_path);

    m_last_job_launched_item = current_item;

    if (m_last_job_launched_item->type() == MatisseTreeItem::Type) {
        MatisseTreeItem *last_job_launched_icon_item = static_cast<MatisseTreeItem *>(m_last_job_launched_item);
        IconizedWidgetWrapper *item_wrapper = new IconizedTreeItemWrapper(last_job_launched_icon_item, 0);
        m_icon_factory->attachIcon(item_wrapper, "lnf/icons/led.svg", false, false, m_colors_by_level.value(WARNING));
    } else {
        qCritical() << QString("Item for job '%1' is not of type MatisseTreeItem, cannot display icon").arg(job_name);
    }


    QString msg = tr("Job %1 running...").arg(job_name);
    showStatusMessage(msg, IDLE);

    doFoldUnfoldParameters(false);
    m_stop_button->setEnabled(true);
    m_ongoing_process_info_label->show();
    m_ongoing_process_completion->show();
    emit si_processRunning();

    bool run_success = m_engine.processJob(*m_current_job);

    if (!run_success) {
        QString msg = tr("Error %1: %2").arg(job_name).arg(m_engine.messageStr());
        showStatusMessage(msg, ERROR);
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

  checkRemoteDirCreated();

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

        resetOngoingProcessIndicators();;
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


void MainGui::sl_jobProcessed(QString _name, bool _is_cancelled) {
    //qDebug() << "Job done : " << name;

    if (!m_engine.errorFlag()) {
        ProcessDataManager* process_data_manager = ProcessDataManager::instance();

        JobDefinition *job_def = process_data_manager->getJob(_name);
        QDateTime now = QDateTime::currentDateTime();
        job_def->executionDefinition()->setExecutionDate(now);
        process_data_manager->writeJobFile(job_def, true);
        if (m_last_job_launched_item) {
            if (_is_cancelled) {
                updateJobStatus(_name, m_last_job_launched_item, IDLE, tr("Job %1 cancelled..."));

                // TODO Image cleaning ?
            }
            else  {
                updateJobStatus(_name, m_last_job_launched_item, OK, tr("Job %1 finished..."));

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
    /* case assembly was modified (parameters or processing chain) : name is suffixed by '*' */
    if (assembly_name.right(1) == "*") {
        assembly_name.chop(1); // remove traing '*'
    } else {
        qWarning() << QString("New assembly name '%1' does not end with '*'").arg(assembly_name);
    }

    return assembly_name;
}

} // namespace matisse


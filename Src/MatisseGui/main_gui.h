#ifndef MATISSE_MAIN_GUI_H_
#define MATISSE_MAIN_GUI_H_

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

#include "element_widget_provider.h"
#include "assembly_graphics_scene.h"
#include "key_value_list.h"
#include "matisse_engine.h"
#include "graphical_charter.h"
#include "assembly_helper.h"
#include "job_commons.h"
#include "job_helper.h"
#include "import_export_helper.h"
#include "preferences_dialog.h"
#include "data_viewer.h"
#include "assembly_editor.h"
#include "dim2_file_reader.h"
#include "status_message_widget.h"
#include "matisse_preferences.h"
#include "home_widget.h"
#include "matisse_menu.h"
#include "live_process_wheel.h"
#include "about_dialog.h"
#include "system_data_manager.h"
#include "process_data_manager.h"
#include "string_utils.h"
#include "matisse_icon_factory.h"
#include "iconized_button_wrapper.h"
#include "iconized_label_wrapper.h"
#include "iconized_tree_item_wrapper.h"
#include "matisse_tree_item.h"
#include "welcome_dialog.h"
#include "camera_manager_tool.h"
#include "camera_calib_dialog.h"
#include "remote_job_helper.h"


namespace Ui {
class MainGui;
}

namespace matisse {

enum eUserAction {
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
        m_last_action_performed(SYSTEM_INIT) {}
    eUserAction lastActionPerformed() const { return m_last_action_performed; }
    void setLastActionPerformed(eUserAction _last_action_performed) {
        qDebug() << "Last action performed : " << _last_action_performed;
        m_last_action_performed = _last_action_performed;
    }

private:
    eUserAction m_last_action_performed;
};

class MainGui : public QMainWindow, ElementWidgetProvider
{
    Q_OBJECT
    
public:
    explicit MainGui(QWidget *_parent = 0);
    ~MainGui();
    void init();
    void loadDefaultStyleSheet();

    virtual SourceWidget * getSourceWidget(QString _name);
    virtual ProcessorWidget * getProcessorWidget(QString _name);
    virtual DestinationWidget * getDestinationWidget(QString _name);

    void applyNewApplicationContext();
    void handleAssemblyModified();
    void checkAndSelectAssembly(QString _selected_assembly_name);
    void checkAndSelectJob(QTreeWidgetItem* _selected_item);
    void resetOngoingProcessIndicators();
    void updatePreferredDatasetParameters();

    void setAssemblyHelper(AssemblyHelper *_assembly_helper);
    void setJobHelper(JobHelper *_job_helper);
    void setImportExportHelper(ImportExportHelper *_import_export_helper);
    void setRemoteJobHelper(RemoteJobHelper *_remote_job_helper);

    void initMapFeatures();
private:
    Ui::MainGui *m_ui;
    bool m_is_map_view;
    MatisseEngine m_engine;
    UserActionContext m_context;

    AssemblyHelper *m_assembly_helper;
    JobHelper *m_job_helper;
    ImportExportHelper *m_import_export_helper;
    RemoteJobHelper *m_remote_job_helper;

    QString m_app_version;

    MatissePreferences* m_preferences;
    QTranslator* m_tools_translator_en;
    QTranslator* m_tools_translator_fr;
    QTranslator* m_server_translator_en;
    QTranslator* m_server_translator_fr;
    QString m_current_language;

    bool m_job_parameter_modified;
    bool m_is_assembly_modified;
    bool m_is_assembly_complete;

    static const QString PREFERENCES_FILEPATH;
    static const QString JOB_REMOTE_PREFIX;
    static const QString DEFAULT_RESULT_PATH;
    static const QString DEFAULT_MOSAIC_PREFIX;

    QTreeWidgetItem * m_last_job_launched_item;
    AssemblyDefinition *m_new_assembly;
    AssemblyDefinition *m_current_assembly;
    JobDefinition *m_current_job;
    DataViewer * m_data_viewer;
    AssemblyEditor * m_assembly_editor;
    QScrollArea * m_parameters_dock;
    ParametersWidgetSkeleton * m_parameters_widget;
    QLabel* m_messages_picto;

    QString m_current_remote_execution_bundle;

    QTreeWidgetItem *m_assembly_version_property_item;
    QTreeWidgetItem *m_assembly_creation_date_property_item;
    QTreeWidgetItem *m_assembly_author_property_item;
    QTreeWidgetItem *m_assembly_comment_property_header_item;
    QTreeWidgetItem *m_assembly_comment_property_item;
    QLabel *m_assembly_comment_property_item_text;

    eApplicationMode m_active_application_mode;
    QHash<QString, QTreeWidgetItem*> m_assemblies_items;
    QHash<QString, KeyValueList*> m_assemblies_properties;
    QMap<eApplicationMode, QString> m_stylesheet_by_app_mode;
    QMap<eApplicationMode, QString> m_wheel_colors_by_mode;
    QMap<eApplicationMode, QString> m_colors_by_mode1;
    QMap<eApplicationMode, QString> m_colors_by_mode2;
    QMap<eJobStatusMessageLevel, QString> m_colors_by_level;
    QToolButton* m_visu_mode_button;
    QToolButton* m_stop_button;
    QToolButton* m_maximize_or_restore_button;
    QToolButton* m_close_button;
    QToolButton* m_minimize_button;
    QToolButton* m_home_button;
    QPushButton* m_reset_messages_button;
    HomeWidget *m_home_widget;
    WelcomeDialog *m_welcome_dialog;

    CameraManagerTool m_camera_manager_tool_dialog;
    CameraCalibDialog m_camera_calib_tool_dialog;

    bool m_is_night_display_mode;
    QMap<QString, QString> m_current_color_set;

    QLabel *m_active_view_or_mode_label;
    QLabel *m_current_date_time_label;
    QTimer *m_date_time_timer;
    QLabel *m_ongoing_process_info_label;
    QLabel *m_matisse_version_label;
    QProgressBar *m_ongoing_process_completion;
    LiveProcessWheel *m_live_process_wheel;

    // status bar
    StatusMessageWidget* m_status_message_widget;

    QHash<QString, SourceWidget *> m_available_sources;
    QHash<QString, ProcessorWidget *> m_available_processors;
    QHash<QString, DestinationWidget *> m_available_destinations;

    MatisseIconFactory *m_icon_factory;
    IconizedWidgetWrapper *m_max_or_restore_button_wrapper;
    IconizedWidgetWrapper *m_visu_mode_button_wrapper;

    /* static menu headers */
    MatisseMenu *m_file_menu;
    MatisseMenu *m_display_menu;
    MatisseMenu *m_process_menu;
    MatisseMenu *m_tool_menu;
    MatisseMenu *m_help_menu;

    /* static menu actions */
    QAction* m_export_map_view_act;
    QAction* m_close_act;
    QAction* m_day_night_mode_act;
    QAction* m_map_toolbar_act;
    QAction* m_create_assembly_act;
    QAction* m_save_assembly_act;
    QAction* m_import_assembly_act;
    QAction* m_export_assembly_act;
    QAction* m_app_config_act;
    QAction* m_preprocessing_tool;
    QAction* m_nmea_nav_extrator_tool;
    QAction* m_camera_manager_tool;
    QAction* m_camera_calib_tool;
    QAction* m_video_to_image_tool_act;
    QAction* m_user_manual_act;
    QAction* m_about_act;

    /* assembly context menu */
    QAction* m_create_job_act;
    QAction* m_import_job_act;
    QAction* m_delete_assembly_act;
    QAction* m_restore_job_act;
    QAction* m_clone_assembly_act;
    QAction* m_update_assembly_properties_act;

    /* job context menu */
    QAction* _executeJobAct;
    QAction* m_execute_remote_job_act;
    QAction* m_upload_data_act;
    QAction* m_select_remote_data_act;
    QAction* m_download_job_results_act;
    QAction* m_save_job_act;
    QAction* m_clone_job_act;
    QAction* m_export_job_act;
    QAction* m_delete_job_act;
    QAction* m_archive_job_act;
    QAction* m_go_to_results_act;

private:
    void dpiScaleWidgets();
    void initMainMenu();
    void initIconFactory();
    void initStylesheetSelection();
    void initContextMenus();
    void enableActions();
    void initDateTimeDisplay();
    void initPreferences();
    void initVersionDisplay();
    void loadAssemblyParameters(AssemblyDefinition *_selected_assembly);
    void initParametersWidget();
    void initProcessorWidgets();
    void lookupChildWidgets();
    void initProcessWheelSignalling();
    void initUserActions();
    void initEngine();
    void initAssemblyHelper();
    void initJobHelper();
    void initImportExportHelper();
    void initRemoteJobHelper();
    void initAssemblyCreationScene();
    void initWelcomeDialog();
    void loadAssembliesAndJobsLists(bool _do_expand=true);
    void displayAssembly(QString _assembly_name);
    void displayJob(QString _job_name, bool _force_reload = false);
    void selectJob(QString _job_name, bool _reload_job = true);
    void selectAssembly(QString _assembly_name, bool _reload_assembly = true);
    QTreeWidgetItem * addAssemblyInTree(AssemblyDefinition *_assembly);
    QTreeWidgetItem * addJobInTree(JobDefinition *_job, bool _is_new_job = false);

    void loadStyleSheet(eApplicationMode _mode);

    void saveAssemblyAndReload(AssemblyDefinition *_assembly);
    void displayAssemblyProperties(AssemblyDefinition *_selected_assembly);

    void initStatusBar();
    void showStatusMessage(QString _message = "", eJobStatusMessageLevel _level = IDLE);

    void initLanguages();
    void updateLanguage(QString _language, bool _force_retranslation = false);
    void retranslate();
    
    bool loadResultToCartoView(QString _result_file_p, bool _remove_previous_scenes=true);
    
    void doFoldUnfoldParameters(bool _do_unfold, bool _is_explicit_action = false);

    void freezeJobUserAction(bool _freeze_p);

    void handleJobModified();
    QString getActualAssemblyOrJobName(QTreeWidgetItem* _current_item);
    QString getActualNewAssemblyName();
    bool promptAssemblyNotSaved();
    void promptJobNotSaved();

    void deleteAssemblyAndReload(bool _prompt_user);

    void executeImportWorkflow(bool _is_job_import_action = false);
    void executeExportWorkflow(bool _is_job_export_action, bool _is_for_remote_execution = false);
    void updateJobStatus(QString _job_name, QTreeWidgetItem* _item,
                         eJobStatusMessageLevel _indicator, QString _message);

protected:
    void changeEvent(QEvent *_event); // overriding event handler for dynamic translation

protected slots:
    void sl_saveAssembly();
    void sl_deleteAssembly();
    void sl_newJob();
    void sl_saveJob();
    void sl_deleteJob();
    void sl_assemblyContextMenuRequested(const QPoint &_pos);

    void sl_maximizeOrRestore();
    void sl_quit();
    void sl_moveWindow(const QPoint &_pos);

    void sl_clearAssembly();
    void sl_newAssembly();
    void sl_swapMapOrCreationView();
    void sl_launchJob();
    void sl_uploadJobData();
    void sl_selectRemoteJobData();
    void sl_launchRemoteJob();
    void sl_downloadJobResults();
    void sl_onRemoteJobResultsReceived(QString _job_name);
    void sl_stopJob();
    void sl_jobShowImageOnMainView(QString _name, Image *_image);
    void sl_userInformation(QString _user_text);
    void sl_processCompletion(quint8 _percent_complete);
    void sl_showInformationMessage(QString _title, QString _message);
    void sl_showErrorMessage(QString _title, QString _message);
    void sl_jobProcessed(QString _job_name, bool _is_cancelled);
    void sl_assembliesReload();
    void sl_modifiedParameters(bool _changed);
    void sl_modifiedAssembly();
    void sl_assemblyComplete(bool _is_complete);
    void sl_selectAssemblyOrJob(QTreeWidgetItem *_selected_item, int _column=0);
    void sl_updateTimeDisplay();
    void sl_updatePreferences();
    void sl_foldUnfoldParameters();
    void sl_showUserManual();
    void sl_showAboutBox();
    void sl_exportAssembly();
    void sl_importAssembly();
    void sl_exportJob();
    void sl_importJob();
    void sl_goToResult();
    void sl_archiveJob();
    void sl_restoreJobs();
    void sl_duplicateJob();
    void sl_duplicateAssembly();
    void sl_swapDayNightDisplay();
    void sl_exportMapToImage();
    void sl_launchPreprocessingTool();
    void sl_launchNmeaExtractorTool();
    void sl_launchCameraManagerTool();
    void sl_launchCameraCalibTool();

public slots:
    void sl_showApplicationMode(eApplicationMode _mode);
    void sl_goHome();
    void sl_show3DFileOnMainView(QString _filepath_p);
    void sl_addRasterFileToMap(QString _filepath_p);
    void sl_addToLog(QString _loggin_text);

signals:
    void si_processRunning();
    void si_processStopped();
    void si_processFrozen();
    void si_updateWheelColors(QString _colors);
    void si_updateColorPalette(QMap<QString,QString>);
    void si_updateExecutionStatusColor(QString _new_status_color_alias);
    void si_updateAppModeColors(QString _new_app_mode_color_alias1, QString _new_app_mode_color_alias2);
};

} // namespace matisse

#endif // MATISSE_MAIN_GUI_H_

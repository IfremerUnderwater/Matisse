#ifndef MATISSE_MATISSE_ENGINE_H_
#define MATISSE_MATISSE_ENGINE_H_

#include <QString>
#include <QTcpSocket>
#include <QThread>
#include <QTcpServer>
#include <QNetworkInterface>
#include <QModelIndex>
#include <QCryptographicHash>
#include <QFile>

#include "processor.h"
#include "input_data_provider.h"
#include "output_data_writer.h"
#include "job_server.h"
#include "file_utils.h"
#include "matisse_parameters_manager.h"
#include "system_data_manager.h"
#include "process_data_manager.h"


namespace matisse {

enum eApplicationMode {
    PROGRAMMING,
    REAL_TIME,
    POST_PROCESSING,
    APP_CONFIG
};

class MatisseEngine;
class JobTask : public QObject{
    Q_OBJECT

public:
    explicit JobTask(InputDataProvider *_input_data_provider, QList<Processor *> _processors, OutputDataWriter *_output_data_writer , JobDefinition *_job_definition, MatisseParameters *_parameters);
    virtual ~JobTask();
    void stop(bool _cancel=false);
    JobDefinition *jobDefinition() const;

    QStringList resultFileNames() const;

    bool isCancelled() const;

    void setJobLauncher(QObject *_job_launcher);
    void setIsServerMode(bool _is_server_mode);

signals:
    void si_jobShowImageOnMainView(QString _job_name, Image *_image);
    void si_userInformation(QString _user_text);
    void si_processCompletion(quint8 _percent_complete);
    void si_jobStopped();

public slots:
    void sl_start();
    void sl_stop();
    void sl_showImageOnMainView(Image *_image);
    void sl_userInformation(QString _user_text);
    void sl_processCompletion(quint8 _percent_complete);
    void sl_fatalError();
    void sl_logToFile(QString _log_info);

private:
    QFile* m_user_log_file;
    QObject* m_job_launcher;
    Context* m_context;
    InputDataProvider* m_input_data_provider;
    QList<Processor*> m_processors;
    OutputDataWriter* m_output_data_writer;
    JobDefinition *m_job_definition;
    MatisseParameters *m_mat_parameters;
    MatisseParameters *m_assembly_parameters;
    QStringList m_result_file_names;
    volatile bool m_is_cancelled;
    bool m_is_server_mode = false;
    bool m_log_file_opened = false;
};

class MatisseEngine : public QObject
{
    Q_OBJECT

public:
    explicit MatisseEngine(QObject *_parent = 0, bool _is_server_mode=false);
    virtual ~MatisseEngine();

    void init();
    void setJobLauncher(QObject* _job_launcher);

    QList<Processor*> const getAvailableProcessors();
    QList<InputDataProvider*> const getAvailableInputDataProviders();
    QList<OutputDataWriter*> const getAvailableOutputDataWriters();

    void addParametersForInputDataProvider(QString _name);
    void addParametersForProcessor(QString _name);
    void addParametersForOutputDataWriter(QString _name);

    bool removeModuleAndExpectedParameters(QString _name);

    bool processJob(JobDefinition*  _job_definition);
    bool isProcessingJob();
    bool stopJob(bool _cancel=false);
    bool errorFlag();
    QString messageStr();
    MatisseParametersManager * parametersManager() { return m_dico_param_mgr; }

signals:
    void si_jobShowImageOnMainView(QString _job_name, Image *_image);
    void si_userInformation(QString _user_text);
    void si_processCompletion(quint8 _percent_complete);
    void si_jobProcessed(QString _job_name, bool _is_cancelled);

private slots:
    void sl_currentJobProcessed();

private:
    MatisseParameters* buildMatisseParameters(JobDefinition *_job);
    bool buildJobTask( AssemblyDefinition *_assembly, JobDefinition *_job_definition, MatisseParameters *_matisse_parameters);
    void setMessageStr(QString _message_Str = "", bool _error = true);
    bool loadParametersDictionnary();

    bool m_is_server_mode;
    QObject* m_job_launcher;
    JobServer *m_job_server;
    JobTask* m_current_job;
    QThread* m_thread;
    MatisseParametersManager* m_dico_param_mgr;
    QHash<QString, Processor*> m_processors;
    QHash<QString, InputDataProvider*> m_input_data_providers;
    QHash<QString, OutputDataWriter*> m_output_data_writers;
    QHash<QString, QList<MatisseParameter> > m_expected_parameters_by_module;
    QString m_message_str;
    bool m_error_flag;
};

} // namespace matisse


#endif // MATISSE_MATISSE_ENGINE_H_

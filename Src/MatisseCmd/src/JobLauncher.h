#ifndef ASSEMBLYGUI_H
#define ASSEMBLYGUI_H

#include <QDateTime>
#include <QFile>
#include <QDir>
#include <QPair>
#include <QDesktopServices>
#include <QRegExp>

#include <QtDebug>

#include "KeyValueList.h"
#include "Server.h"
#include "Dim2FileReader.h"
#include "MatissePreferences.h"
#include "SystemDataManager.h"
#include "ProcessDataManager.h"
#include "PlatformComparisonStatus.h"
#include "StringUtils.h"


namespace MatisseServer {


class JobLauncher : public QObject
{
    Q_OBJECT
    
public:
    explicit JobLauncher(QObject *parent = 0);
    ~JobLauncher();

    void init();

    void setSystemDataManager(SystemDataManager *systemDataManager);
    void setProcessDataManager(ProcessDataManager *processDataManager);

    void launchJob(QString jobName);

private:
    Server _server;

    SystemDataManager *_systemDataManager;
    ProcessDataManager *_processDataManager;

    AssemblyDefinition *_newAssembly;
    AssemblyDefinition *_currentAssembly;
    JobDefinition *_currentJob;



private:
    void initServer();


signals:
    void signal_processRunning();
    void signal_processStopped();
    void signal_processFrozen();
};
}

#endif // ASSEMBLYGUI_H

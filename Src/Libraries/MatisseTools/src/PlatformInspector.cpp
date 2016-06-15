#include "PlatformInspector.h"

#include "PlatformQgisComponent.h"
#include "PlatformOpencvComponent.h"
#include "PlatformOsgComponent.h"

using namespace MatisseTools;

PlatformInspector::PlatformInspector() :
    _dump(NULL)
{    
}

PlatformInspector::~PlatformInspector()
{
    if (_dump) {
        delete _dump;

        foreach (PlatformComponent *comp, _components) {
            _components.removeOne(comp);
            delete comp;
        }
    }
}

void PlatformInspector::init()
{
    if (!_dump) {
        _dump = new PlatformDump();

        _components.append(new PlatformQGisComponent());
        _components.append(new PlatformOpencvComponent());
#ifdef WITH_OSG
        _components.append(new PlatformOsgComponent());
#endif
    }
}

void PlatformInspector::inspect()
{
    inspectOsAndEnvVariables();

    /* Retrieving information version for each component */
    foreach (PlatformComponent *comp, _components) {
        QString compName = comp->getName();
        QString compVersion = comp->getVersionInfo();
        _dump->addComponentInfo(compName, compVersion);
    }
}

void PlatformInspector::inspectOsAndEnvVariables()
{
    QString osName = "Unknown";
    QString osVersion = "N/A";
    QString envVariables = "";

    QString versionCommand = "";
    QString envVarCommand = "";

    /* Resolving shell commands according to OS */
#ifdef Q_OS_WIN
    osName = "Windows";
    versionCommand = "cmd /C ver"; // opening Windows console displays OS version
    envVarCommand = "cmd /C set";
#elif Q_OS_LINUX
    osName = "Linux";
    versionCommand = "/bin/sh -c \"uname -sor\"";
    envVarCommand = "/bin/sh -c \"env\"";
#elif Q_OS_DARWIN // Mac
    osName = "Mac OS";
    versionCommand = "/bin/sh -c \"sw_vers -productVersion\"";
    envVarCommand = "/bin/sh -c \"env\"";
#else
    QWarning() << "Unhandled OS : cannot retrive version information";
#endif

    if (!versionCommand.isEmpty()) {
        qDebug() << "Inspecting OS version...";
        QProcess cmd;
        cmd.start(versionCommand);
        cmd.waitForBytesWritten(100);
        /* sequence avoids QProcess warning display */
        if (!cmd.waitForFinished(100)) {
            cmd.kill();
            cmd.waitForFinished(1);
        }
        QString cmdOut = cmd.readAllStandardOutput();
        QStringList cmdOutLines = cmdOut.split("\n");
        // Take the first non empty line
        foreach (QString cmdOutLine, cmdOutLines) {
            if (!cmdOutLine.trimmed().isEmpty()) {
                osVersion = cmdOutLine.simplified();
            }
        }
    }

    if (!envVarCommand.isEmpty()) {
        qDebug() << "Inspecting environment variables...";
        QProcess cmd;
        cmd.start(envVarCommand);
        /* sequence avoids QProcess warning display */
        cmd.waitForBytesWritten(100);
        if (!cmd.waitForFinished(100)) {
            cmd.kill();
            cmd.waitForFinished(1);
        }
        QString cmdOut = cmd.readAllStandardOutput();
        envVariables = cmdOut;
    }

    _dump->setOsName(osName);
    _dump->setOsVersion(osVersion);
    _dump->setEnvVariables(envVariables);
}

PlatformDump *PlatformInspector::getDump() const
{
    return _dump;
}



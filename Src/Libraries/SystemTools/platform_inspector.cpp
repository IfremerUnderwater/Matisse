#include "platform_inspector.h"

#include "platform_opencv_component.h"
#include "platform_osg_component.h"

namespace system_tools {

PlatformInspector::PlatformInspector() :
    m_dump(NULL)
{
}

PlatformInspector::~PlatformInspector()
{
    if (m_dump) {
        delete m_dump;

        foreach (PlatformComponent *comp, m_components) {
            m_components.removeOne(comp);
            delete comp;
        }
    }
}

void PlatformInspector::init()
{
    if (!m_dump) {
        m_dump = new PlatformDump();

        //_components.append(new PlatformQGisComponent());
        m_components.append(new PlatformOpencvComponent());
        m_components.append(new PlatformOsgComponent());
    }
}

void PlatformInspector::inspect()
{
    inspectOsAndEnvVariables();

    /* Retrieving information version for each component */
    foreach (PlatformComponent *comp, m_components) {
        QString comp_name = comp->getName();
        QString comp_version = comp->getVersionInfo();
        m_dump->addComponentInfo(comp_name, comp_version);
    }
}

void PlatformInspector::inspectOsAndEnvVariables()
{
    QString os_name = "Unknown";
    QString os_version = "N/A";
    QString env_variables = "";

    QString version_command = "";
    QString env_var_command = "";

    /* Resolving shell commands according to OS */
#ifdef Q_OS_WIN
    os_name = "Windows";
    version_command = "cmd /C ver"; // opening Windows console displays OS version
    env_var_command = "cmd /C set";
#elif defined(Q_OS_LINUX)
    os_name = "Linux";
    version_command = "/bin/sh -c \"uname -sor\"";
    env_var_command = "/bin/sh -c \"env\"";
#elif defined(Q_OS_DARWIN) // Mac
    os_name = "Mac OS";
    version_command = "/bin/sh -c \"sw_vers -productVersion\"";
    env_var_Command = "/bin/sh -c \"env\"";
#else
    QWarning() << "Unhandled OS : cannot retrive version information";
#endif

    if (!version_command.isEmpty()) {
        qDebug() << "Inspecting OS version...";
        QProcess cmd;
        cmd.start(version_command);
        cmd.waitForBytesWritten(100);
        /* sequence avoids QProcess warning display */
        if (!cmd.waitForFinished(100)) {
            cmd.kill();
            cmd.waitForFinished(1);
        }
        QString cmd_out = cmd.readAllStandardOutput();
        QStringList cmd_out_lines = cmd_out.split("\n");
        // Take the first non empty line
        foreach (QString cmdOutLine, cmd_out_lines) {
            if (!cmdOutLine.trimmed().isEmpty()) {
                os_version = cmdOutLine.simplified();
            }
        }
    }

    if (!env_var_command.isEmpty()) {
        qDebug() << "Inspecting environment variables...";
        QProcess cmd;
        cmd.start(env_var_command);
        /* sequence avoids QProcess warning display */
        cmd.waitForBytesWritten(100);
        if (!cmd.waitForFinished(100)) {
            cmd.kill();
            cmd.waitForFinished(1);
        }
        QString cmd_out = cmd.readAllStandardOutput();
        env_variables = cmd_out;
    }

    m_dump->setOsName(os_name);
    m_dump->setOsVersion(os_version);
    m_dump->setEnvVariables(env_variables);
}

PlatformDump *PlatformInspector::getDump() const
{
    return m_dump;
}

} // namespace system_tools


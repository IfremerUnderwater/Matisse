#ifndef NETWORK_TOOLS_NETWORK_ACTION_H_
#define NETWORK_TOOLS_NETWORK_ACTION_H_

#include <QDir>
#include <QObject>

namespace network_tools {

enum class FileTypeFilter {
  Dirs = 0x001,
  Files = 0x002,
  AllEntries = Dirs | Files
};
Q_DECLARE_FLAGS(FileTypeFilters, FileTypeFilter)
Q_DECLARE_OPERATORS_FOR_FLAGS(FileTypeFilters)

class NetworkAction : public QObject
{
    Q_OBJECT

public:
    enum class NetworkActionType { UploadFile, DownloadFile, UploadDir, ListDirContent, DownloadDir, SendCommand };
    Q_ENUM(NetworkActionType)

    NetworkAction(NetworkActionType _type);

    NetworkActionType type() { return m_type; }
    bool isValid() { return m_is_valid; }
    bool isTerminated() { return m_is_terminated; }
    QString metainfo() { return m_metainfo; }
    void setMetainfo(QString _metainfo) { m_metainfo = _metainfo; }

    virtual void init() = 0;
    virtual void execute() = 0;
    virtual QString progressMessage() = 0;
void terminate();

protected:
    virtual void doTerminate()=0;

    bool m_is_valid = true;
    bool m_is_terminated = false;
    QString m_metainfo;

private:
    NetworkActionType m_type;
};

} // namespace network_tools

#endif  // NETWORK_TOOLS_NETWORK_ACTION_H_

#ifndef NETWORK_TOOLS_NETWORK_ACTION_H_
#define NETWORK_TOOLS_NETWORK_ACTION_H_

#include "network_commons.h"

#include <QDir>
#include <QObject>

namespace network_tools {


class NetworkAction : public QObject
{
    Q_OBJECT

public:
    enum class eNetworkActionType { UploadFile, DownloadFile, UploadDir, ListDirContent, DownloadDir, SendCommand };
    Q_ENUM(eNetworkActionType)

    NetworkAction(eNetworkActionType _type);

    eNetworkActionType type() { return m_type; }
    bool isValid() { return m_is_valid; }
    bool isTerminated() { return m_is_terminated; }
    QString metainfo() { return m_meta_info; }
    void setMetaInfo(QString _meta_info) { m_meta_info = _meta_info; }

    virtual void init() = 0;
    virtual void execute() = 0;
    virtual QString progressMessage() = 0;
void terminate();

protected:
    virtual void doTerminate()=0;

    bool m_is_valid = true;
    bool m_is_terminated = false;
    QString m_meta_info;

private:
    eNetworkActionType m_type;
};

} // namespace network_tools

#endif  // NETWORK_TOOLS_NETWORK_ACTION_H_

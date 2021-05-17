#include "network_action_dir_content.h"

#include <QRegularExpression>

namespace MatisseCommon {

NetworkActionDirContent::NetworkActionDirContent(QString _remote_dir,
                                                 FileTypeFilters _flags,
                                                 QStringList _file_filters) :
    NetworkFileAction(NetworkActionType::ListDirContent)
{
    m_remote_dir = _remote_dir;
    m_flags = _flags;

    /* check filters */
    for (QString filter : _file_filters) {
        QRegularExpression filter_rexp("\\*\\..+");
        QRegularExpressionMatch match = filter_rexp.match(filter);

        if (!match.hasMatch()) {
            qWarning() << QString(
                          "ListDirContentAction: filter '%1' is not in the "
                          "format '*.ext' and will be ignored")
                          .arg(filter);
            continue;
        }

        qDebug() << "ListDirContentAction: add file filter " << filter;
        m_file_filters.append(filter);
    }
}

void NetworkActionDirContent::init()
{
    emit si_initFileChannel();
}

void NetworkActionDirContent::execute()
{
    emit si_dirContent(m_remote_dir, m_flags, m_file_filters);
}

QString NetworkActionDirContent::progressMessage()
{
    return QString("Listing contents for dir '%1'").arg(m_remote_dir);
}

void NetworkActionDirContent::doTerminate()
{

}

} // namespace MatisseCommon

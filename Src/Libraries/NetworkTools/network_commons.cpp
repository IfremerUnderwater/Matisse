#include "network_commons.h"

namespace network_tools
{

NetworkFileInfo::NetworkFileInfo(QString _name, bool _is_dir, quint64 _size, QDateTime _last_modified)
    : m_name(_name), m_is_dir(_is_dir), m_size(_size), m_last_modified(_last_modified)
{

}


} // namespace network_tools

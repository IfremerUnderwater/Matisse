#include "assembly_definition.h"

namespace matisse {

AssemblyDefinition::AssemblyDefinition(QObject *_parent) :
    QObject(_parent),
    m_source_definition(NULL),
    m_destination_definition(NULL)
{
}

QString AssemblyDefinition::version() const
{
    return m_version;
}

void AssemblyDefinition::setVersion(const QString &_version)
{
    m_version = _version;
}

void AssemblyDefinition::addProcessorDef(ProcessorDefinition *_processor_def)
{
    m_processor_defs.append(_processor_def);
}


void AssemblyDefinition::addConnectionDef(ConnectionDefinition *_connection_def)
{
    m_connection_defs.append(_connection_def);
}

SourceDefinition* AssemblyDefinition::sourceDefinition() const
{
    return m_source_definition;
}

void AssemblyDefinition::setSourceDefinition(SourceDefinition *_source_definition)
{
    m_source_definition = _source_definition;
}

DestinationDefinition * AssemblyDefinition::destinationDefinition() const
{
    return m_destination_definition;
}

void AssemblyDefinition::setDestinationDefinition(DestinationDefinition *_destination_definition)
{
    m_destination_definition = _destination_definition;
}

QString AssemblyDefinition::filename() const
{
    return m_filename;
}

void AssemblyDefinition::setFilename(const QString &_filename)
{
    m_filename = _filename;
}
QString AssemblyDefinition::name() const
{
    return m_name;
}

void AssemblyDefinition::setName(const QString &_name)
{
    m_name = _name;
}
bool AssemblyDefinition::usable() const
{
    return m_usable;
}

void AssemblyDefinition::setUsable(bool _usable)
{
    m_usable = _usable;
}


QDate AssemblyDefinition::creationDate() const
{
    return m_creation_date;
}

void AssemblyDefinition::setCreationDate(const QDate &_creation_date)
{
    m_creation_date = _creation_date;
}

QString AssemblyDefinition::date() const
{
    return m_date;
}

void AssemblyDefinition::setDate(const QString &_date)
{
    m_date = _date;
}
QString AssemblyDefinition::author() const
{
    return m_author;
}

void AssemblyDefinition::setAuthor(const QString &_author)
{
    m_author = _author;
}

QString AssemblyDefinition::comment() const
{
    return m_comment;
}

void AssemblyDefinition::setComment(const QString &_comment)
{
    m_comment = _comment;
}

QList<ProcessorDefinition*> AssemblyDefinition::processorDefs() const
{
    return m_processor_defs;
}

QList<ConnectionDefinition*> AssemblyDefinition::connectionDefs() const
{
    return m_connection_defs;
}

QList<eAssemblyDefinitionValidity> AssemblyDefinition::checkDefinition()
{
    QList<eAssemblyDefinitionValidity> ret_list;

    int source_order = -1;
    QList <quint32> processor_orders;
    quint32 min_processor_order = 4294967295;
    quint32 max_processor_order = 0;
    quint32 destination_order = 0;
    bool duplicated_processor_order = false;

    if (!m_source_definition) {
        ret_list << MISSING_SOURCE;
    } else {
        source_order = m_source_definition->order();
    }
    if (m_processor_defs.size() == 0) {
        ret_list << MISSING_PROCESSOR;
    } else {
        foreach(ProcessorDefinition * processor, m_processor_defs) {
            quint32 process_order = processor->order();
            min_processor_order = qMin(process_order, min_processor_order);
            max_processor_order = qMax(process_order, max_processor_order);
            if (!processor_orders.contains(process_order)) {
                processor_orders << processor->order();
            } else {
                duplicated_processor_order = true;
            }
        }
    }
    if (!m_destination_definition) {
        ret_list << MISSING_DESTINATION;
    } else {
        destination_order = m_destination_definition->order();
    }

    // test des orders
    if ((source_order != 0)
            || duplicated_processor_order
            || (min_processor_order == 0)
            || !(max_processor_order > min_processor_order)
            || (max_processor_order >= destination_order)) {
        ret_list << INVALID_ORDERS;
    }

    if (m_connection_defs.size() == 0) {
        ret_list << MISSING_CONNECTION;
    } else {
        // pour l'instant on a uniquement des infos sur l'ordre, pas sur le nombre de connections...
        // on verifie donc que:
        // 1) la source est au moins reliée à quelque chose (processeur ou destination...)
        // 2) la destination est reliée à quelque chose d'unique (processeur ou source...)
        // 3) les connections sont faites entre des choses qui existent et qu'aucune entrée n'est reliée à plusieurs sorties...
        QMultiHash<quint32, quint32> connections_map;
        bool ok = true;
        foreach(ConnectionDefinition * connection, m_connection_defs) {
            connections_map.insert(connection->startOrder(), connection->endOrder());
        }

        // test source
        if (source_order == 0) {
            if (!connections_map.contains(0)) {
                ok = false;
            } else {
                QList<quint32> src_connections = connections_map.values();
                QList<quint32> src_possible_connections = processor_orders;
                if (destination_order > 0) {
                    src_possible_connections << destination_order;
                }
                foreach (quint32 src_connection, src_connections) {
                   if (!src_possible_connections.contains(src_connection)) {
                       ok = false;
                       break;
                   }
                }
            }
        }

        // test destination
        if (destination_order > 0) {
            QList<quint32> dest_cons = connections_map.keys(destination_order);
            if (dest_cons.size() != 1) {
                ok = false;
            } else {
                QList<quint32> dest_possible_connections = processor_orders;
                dest_possible_connections << 0;
                if (!dest_possible_connections.contains(dest_cons[0])) {
                    ok =false;
                }
            }
        }

        // test connections, certainement redondant avec des test précédents...
        QList<quint32> dest_possible_connections = processor_orders;
        dest_possible_connections << destination_order;

        QStringList comb_list;
        foreach(ConnectionDefinition * connection, m_connection_defs) {
            quint32 start_order = connection->startOrder();
            quint32 end_order = connection->endOrder();
            QString new_comb = QString("%1_%2").arg(end_order).arg(connection->endLine());
            if ((start_order == 0) && (source_order != 0)) {
                ok =false;
                break;
            }
            if (comb_list.contains(new_comb)) {
                ok = false;
                break;
            }
            comb_list << new_comb;
            if ((start_order != 0) && (!processor_orders.contains(start_order))) {
                ok = false;
                break;
            }
            if (!dest_possible_connections.contains(end_order)) {
                ok = false;
                break;
            }
        }

        if (!ok) {
            ret_list << CONNECTION_ERROR;
        }
    }

    return ret_list;

}

AssemblyDefinition *AssemblyDefinition::duplicate(QString _new_name, QString _new_file_name)
{
    AssemblyDefinition *new_assembly = new AssemblyDefinition();
    new_assembly->setName(_new_name);
    new_assembly->setFilename(_new_file_name);
    new_assembly->setVersion(m_version);
    new_assembly->setAuthor(m_author);
    new_assembly->setComment(m_comment);
    new_assembly->setUsable(m_usable);

    QDateTime creation_date = QDateTime::currentDateTime();
    QString ts = creation_date.toString("dd/MM/yyyy");
    new_assembly->setCreationDate(creation_date.date());
    new_assembly->setDate(ts);

    SourceDefinition *source = new SourceDefinition(m_source_definition->name());
    new_assembly->setSourceDefinition(source);

    foreach (ProcessorDefinition *proc_def, m_processor_defs) {
        ProcessorDefinition *processor = new ProcessorDefinition(proc_def->name(), proc_def->order());
        new_assembly->addProcessorDef(processor);
    }


    foreach (ConnectionDefinition *conn_def, m_connection_defs) {
        ConnectionDefinition *connection = new ConnectionDefinition(
                    conn_def->startOrder(),
                    conn_def->startLine(),
                    conn_def->endOrder(),
                    conn_def->endLine(),
                    conn_def->color()
                    );
        new_assembly->addConnectionDef(connection);
    }

    DestinationDefinition *destination = new DestinationDefinition(m_destination_definition->name(), m_destination_definition->order());
    new_assembly->setDestinationDefinition(destination);

    return new_assembly;
}

void AssemblyDefinition::clearAllElements()
{
    if (m_source_definition) {
        delete m_source_definition;
        m_source_definition = NULL;
    }

    if (m_destination_definition) {
        delete m_destination_definition;
        m_destination_definition = NULL;
    }

    foreach (ProcessorDefinition *procDef, m_processor_defs) {
        m_processor_defs.removeOne(procDef);
        delete procDef;
    }

    foreach (ConnectionDefinition *connDef, m_connection_defs) {
        m_connection_defs.removeOne(connDef);
        delete connDef;
    }
}

} // namespace matisse

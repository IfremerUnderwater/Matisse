#include "AssemblyDefinition.h"

using namespace MatisseTools;

AssemblyDefinition::AssemblyDefinition(QObject *parent) :
    QObject(parent)
{
}

QString AssemblyDefinition::version() const
{
    return _version;
}

void AssemblyDefinition::setVersion(const QString &version)
{
    _version = version;
}

void AssemblyDefinition::addProcessorDef(ProcessorDefinition *processorDef)
{
    _processorDefs.append(processorDef);
}


void AssemblyDefinition::addConnectionDef(ConnectionDefinition *connectionDef)
{
    _connectionDefs.append(connectionDef);
}

SourceDefinition* AssemblyDefinition::sourceDefinition() const
{
    return _sourceDefinition;
}

void AssemblyDefinition::setSourceDefinition(SourceDefinition *sourceDefinition)
{
    _sourceDefinition = sourceDefinition;
}

ParameterDefinition* AssemblyDefinition::parametersDefinition() const
{
    return _parametersDefinition;
}

void AssemblyDefinition::setParametersDefinition( ParameterDefinition *parametersDefinition)
{
    _parametersDefinition = parametersDefinition;
}

DestinationDefinition * AssemblyDefinition::destinationDefinition() const
{
    return _destinationDefinition;
}

void AssemblyDefinition::setDestinationDefinition(MatisseTools::DestinationDefinition *destinationDefinition)
{
    _destinationDefinition = destinationDefinition;
}

QString MatisseTools::AssemblyDefinition::filename() const
{
    return _filename;
}

void MatisseTools::AssemblyDefinition::setFilename(const QString &filename)
{
    _filename = filename;
}
QString AssemblyDefinition::name() const
{
    return _name;
}

void AssemblyDefinition::setName(const QString &name)
{
    _name = name;
}
bool AssemblyDefinition::usable() const
{
    return _usable;
}

void AssemblyDefinition::setUsable(bool usable)
{
    _usable = usable;
}


QDate AssemblyDefinition::creationDate() const
{
    return _creationDate;
}

void AssemblyDefinition::setCreationDate(const QDate &creationDate)
{
    _creationDate = creationDate;
}

QString AssemblyDefinition::date() const
{
    return _date;
}

void AssemblyDefinition::setDate(const QString &date)
{
    _date = date;
}
QString AssemblyDefinition::author() const
{
    return _author;
}

void AssemblyDefinition::setAuthor(const QString &author)
{
    _author = author;
}

QString AssemblyDefinition::comment() const
{
    return _comment;
}

void AssemblyDefinition::setComment(const QString &comment)
{
    _comment = comment;
}

QList<ProcessorDefinition*> AssemblyDefinition::processorDefs() const
{
    // TODO copy?
    return _processorDefs;
}

QList<ConnectionDefinition*> AssemblyDefinition::connectionDefs() const
{
    // TODO copy?
    return _connectionDefs;
}

QString AssemblyDefinition::serialized()
{
    QString text;

    text.append(QString("<MatisseAssembly name=\"%1\" usable=\"%2\">\n").arg(name()).arg(usable()));

    text.append(QString("\t<DescriptorFields>\n"));
    text.append(QString("\t\t<Author>\n%1\n\t\t</Author>\n").arg(author()));
    text.append(QString("\t\t<Version>\n%1\n\t\t</Version>\n").arg(version()));
    text.append(QString("\t\t<Comments>\n%1\n\t\t</Comments>\n").arg(comment()));
    text.append(QString("\t\t<Date>\n%1\n\t\t</Date>\n").arg(creationDate().toString("dd/MM/yyyy")));
    text.append(QString("\t</DescriptorFields>\n\n"));

    if (parametersDefinition()) {
        text.append(QString("\t<Parameters id=\"%1\" model=\"%2\" name=\"%3\">\n\n")
                    .arg(parametersDefinition()->id())
                    .arg(parametersDefinition()->model())
                    .arg(parametersDefinition()->name()));
    }

    if (sourceDefinition()) {
        text.append(QString("\t<Source id=\"%1\" name=\"%2\" order=\"%3\">\n\n")
                    .arg(sourceDefinition()->id())
                    .arg(sourceDefinition()->name())
                    .arg(sourceDefinition()->order()));
    }

    if (processorDefs().size()) {
        text.append(QString("\t<Processors>\n"));
        foreach(ProcessorDefinition * proc, processorDefs()) {
            text.append(QString("\t\t<Processor id=\"%1\" name=\"%2\" order=\"%3\">\n")
                        .arg(proc->id())
                        .arg(proc->name())
                        .arg(proc->order()));
        }
        text.append(QString("\t</Processors>\n"));
    }

    if (_destinationDefinition) {
            text.append(QString("\t<Destination id=\"%1\" name=\"%2\" order=\"%3\">\n")
                        .arg(_destinationDefinition->id())
                        .arg(_destinationDefinition->name())
                        .arg(_destinationDefinition->order()));
    }

    if (connectionDefs().size()) {
        text.append(QString("\t<Connections>\n"));
        foreach(ConnectionDefinition * con, connectionDefs()) {
            text.append(QString("\t\t<Connection startOrder=\"%1\" startLine=\"%2\" endOrder=\"%3\" endLine=\"%4\" color=\"%5\">\n")
                        .arg(con->startOrder())
                        .arg(con->startLine())
                        .arg(con->endOrder())
                        .arg(con->endLine())
                        .arg(con->color()));
        }
        text.append(QString("\t</Connections>\n"));
    }
    return text;
}

QList<AssemblyDefinitionValidity> AssemblyDefinition::checkDefinition()
{
    QList<AssemblyDefinitionValidity> retList;

    int sourceOrder = -1;
    QList <quint32> processorOrders;
    quint32 minProcessorOrder = 4294967295;
    quint32 maxProcessorOrder = 0;
    int destinationOrder = -1;
    bool duplicatedProcessorOrder = false;

    if (!_parametersDefinition) {
        retList << MISSING_PARAMETERS;
    }

    if (!_sourceDefinition) {
        retList << MISSING_SOURCE;
    } else {
        sourceOrder = _sourceDefinition->order();
    }
    if (_processorDefs.size() == 0) {
        retList << MISSING_PROCESSOR;
    } else {
        foreach(ProcessorDefinition * processor, _processorDefs) {
            quint32 processOrder = processor->order();
            minProcessorOrder = qMin(processOrder, minProcessorOrder);
            maxProcessorOrder = qMax(processOrder, maxProcessorOrder);
            if (!processorOrders.contains(processOrder)) {
                processorOrders << processor->order();
            } else {
                duplicatedProcessorOrder = true;
            }
        }
    }
    if (!_destinationDefinition) {
        retList << MISSING_DESTINATION;
    } else {
        destinationOrder = _destinationDefinition->order();
    }

    // test des orders
    if ((sourceOrder != 0)
            || duplicatedProcessorOrder
            || (minProcessorOrder == 0)
            || !(maxProcessorOrder > minProcessorOrder)
            || (maxProcessorOrder >= destinationOrder)) {
        retList << INVALID_ORDERS;
    }

    if (_connectionDefs.size() == 0) {
        retList << MISSING_CONNECTION;
    } else {
        // pour l'instant on a uniquement des infos sur l'ordre, pas sur le nombre de connections...
        // on verifie donc que:
        // 1) la source est au moins reliée à quelque chose (processeur ou destination...)
        // 2) la destination est reliée à quelque chose d'unique (processeur ou source...)
        // 3) les connections sont faites entre des choses qui existent et qu'aucune entrée n'est reliée à plusieurs sorties...
        QMultiHash<quint32, quint32> connectionsMap;
        bool ok = true;
        foreach(ConnectionDefinition * connection, _connectionDefs) {
            connectionsMap.insert(connection->startOrder(), connection->endOrder());
        }

        // test source
        if (sourceOrder == 0) {
            if (!connectionsMap.contains(0)) {
                ok = false;
            } else {
                QList<quint32> srcConnections = connectionsMap.values();
                QList<quint32> srcPossibleConnections = processorOrders;
                if (destinationOrder > 0) {
                    srcPossibleConnections << destinationOrder;
                }
                foreach (quint32 srcConnection, srcConnections) {
                   if (!srcPossibleConnections.contains(srcConnection)) {
                       ok = false;
                       break;
                   }
                }
            }
        }

        // test destination
        if (destinationOrder > -1) {
            QList<quint32> destCons = connectionsMap.keys(destinationOrder);
            if (destCons.size() != 1) {
                ok = false;
            } else {
                QList<quint32> destPossibleConnections = processorOrders;
                destPossibleConnections << 0;
                if (!destPossibleConnections.contains(destCons[0])) {
                    ok =false;
                }
            }
        }

        // test connections, certainement redondant avec des test précédents...
        QList<quint32> destPossibleConnections = processorOrders;
        destPossibleConnections << destinationOrder;

        QStringList combList;
        foreach(ConnectionDefinition * connection, _connectionDefs) {
            quint32 startOrder = connection->startOrder();
            quint32 endOrder = connection->endOrder();
            QString newComb = QString("%1_%2").arg(endOrder).arg(connection->endLine());
            if ((startOrder == 0) && (sourceOrder != 0)) {
                ok =false;
                break;
            }
            if (combList.contains(newComb)) {
                ok = false;
                break;
            }
            combList << newComb;
            if ((startOrder != 0) && (!processorOrders.contains(startOrder))) {
                ok = false;
                break;
            }
            if (!destPossibleConnections.contains(endOrder)) {
                ok = false;
                break;
            }
        }

        if (!ok) {
            retList << CONNECTION_ERROR;
        }
    }

    return retList;

}
bool AssemblyDefinition::isRealTime() const
{
    return _isRealTime;
}

void AssemblyDefinition::setIsRealTime(bool isRealTime)
{
    _isRealTime = isRealTime;
}



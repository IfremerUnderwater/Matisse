#include "assembly_definition.h"

namespace matisse {

AssemblyDefinition::AssemblyDefinition(QObject *parent) :
    QObject(parent),
    _sourceDefinition(NULL),
    _destinationDefinition(NULL)
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

DestinationDefinition * AssemblyDefinition::destinationDefinition() const
{
    return _destinationDefinition;
}

void AssemblyDefinition::setDestinationDefinition(DestinationDefinition *destinationDefinition)
{
    _destinationDefinition = destinationDefinition;
}

QString AssemblyDefinition::filename() const
{
    return _filename;
}

void AssemblyDefinition::setFilename(const QString &filename)
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
    return _processorDefs;
}

QList<ConnectionDefinition*> AssemblyDefinition::connectionDefs() const
{
    return _connectionDefs;
}

QList<AssemblyDefinitionValidity> AssemblyDefinition::checkDefinition()
{
    QList<AssemblyDefinitionValidity> retList;

    int sourceOrder = -1;
    QList <quint32> processorOrders;
    quint32 minProcessorOrder = 4294967295;
    quint32 maxProcessorOrder = 0;
    quint32 destinationOrder = 0;
    bool duplicatedProcessorOrder = false;

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
        if (destinationOrder > 0) {
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

AssemblyDefinition *AssemblyDefinition::duplicate(QString newName, QString newFileName)
{
    AssemblyDefinition *newAssembly = new AssemblyDefinition();
    newAssembly->setName(newName);
    newAssembly->setFilename(newFileName);
    newAssembly->setVersion(_version);
    newAssembly->setAuthor(_author);
    newAssembly->setComment(_comment);
    newAssembly->setUsable(_usable);

    QDateTime creationDate = QDateTime::currentDateTime();
    QString ts = creationDate.toString("dd/MM/yyyy");
    newAssembly->setCreationDate(creationDate.date());
    newAssembly->setDate(ts);

    SourceDefinition *source = new SourceDefinition(_sourceDefinition->name());
    newAssembly->setSourceDefinition(source);

    foreach (ProcessorDefinition *procDef, _processorDefs) {
        ProcessorDefinition *processor = new ProcessorDefinition(procDef->name(), procDef->order());
        newAssembly->addProcessorDef(processor);
    }


    foreach (ConnectionDefinition *connDef, _connectionDefs) {
        ConnectionDefinition *connection = new ConnectionDefinition(
                    connDef->startOrder(),
                    connDef->startLine(),
                    connDef->endOrder(),
                    connDef->endLine(),
                    connDef->color()
                    );
        newAssembly->addConnectionDef(connection);
    }

    DestinationDefinition *destination = new DestinationDefinition(_destinationDefinition->name(), _destinationDefinition->order());
    newAssembly->setDestinationDefinition(destination);

    return newAssembly;
}

void AssemblyDefinition::clearAllElements()
{
    if (_sourceDefinition) {
        delete _sourceDefinition;
        _sourceDefinition = NULL;
    }

    if (_destinationDefinition) {
        delete _destinationDefinition;
        _destinationDefinition = NULL;
    }

    foreach (ProcessorDefinition *procDef, _processorDefs) {
        _processorDefs.removeOne(procDef);
        delete procDef;
    }

    foreach (ConnectionDefinition *connDef, _connectionDefs) {
        _connectionDefs.removeOne(connDef);
        delete connDef;
    }
}

} // namespace matisse

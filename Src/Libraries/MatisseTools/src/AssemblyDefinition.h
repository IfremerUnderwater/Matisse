﻿#ifndef ASSEMBLY_H
#define ASSEMBLY_H

#include <QObject>
#include <QtGlobal>
#include <QList>
#include <QDate>
#include <QRgb>
#include <QMultiHash>
#include <QStringList>

namespace MatisseTools {

enum AssemblyDefinitionValidity {
    VALID = 0,
    MISSING_PARAMETERS,
    MISSING_SOURCE,
    MISSING_PROCESSOR,
    MISSING_DESTINATION,
    MISSING_CONNECTION,
    CONNECTION_ERROR,
    INVALID_ORDERS
};

class SourceDefinition
{
    friend class AssemblyDefinition;
public:
    SourceDefinition(QString name) :
        _id(0),
        _name(name),
        _order(0){}

    quint32 id() const { return _id;}
    QString name() const { return _name;}
    quint32 order() const { return _order;}

private:
    quint32 _id;
    QString _name;
    quint32 _order;
};

class DestinationDefinition
{
    friend class AssemblyDefinition;
public:
    DestinationDefinition(QString name, quint32 order) :
        _id(0),
        _name(name),
        _order(order){}

    quint32 id() const { return _id;}
    QString name() const { return _name;}
    quint32 order() const { return _order;}

private:
    quint32 _id;
    QString _name;
    quint32 _order;
};

class ParameterDefinition
{
    friend class AssemblyDefinition;
public:
    ParameterDefinition(QString model, QString name) :
        _id(0),
        _model(model),
        _name(name) {}

    quint32 id() const { return _id;}
    QString name() const { return _name;}
    QString model() const { return _model;}
    QString filenameWithVersionPath() { return QString("%1/%2.xml").arg(_model).arg(_name.replace(" ", "_")); }

    ParameterDefinition * clone() {
        return new ParameterDefinition(_model, _name);
    }

private:
    quint32 _id;
    QString _model;
    QString _name;
};

class ProcessorDefinition {
    friend class AssemblyDefinition;
public:
    ProcessorDefinition(QString name, quint32 order):
        _id(0),
        _name(name),
        _order(order) {}

    quint32 id() const { return _id;}
    QString name() const { return _name;}
    quint32 order() const { return _order;}

private:
    quint32 _id;
    QString _name;
    quint32 _order;
};

class ConnectionDefinition {
    friend class AssemblyDefinition;
public:
    ConnectionDefinition(quint32 startOrder, quint32 startLine, quint32 endOrder, quint32 endLine, QRgb color) :
        _startOrder(startOrder),
        _startLine(startLine),
        _endOrder(endOrder),
        _endLine(endLine),
        _color(color){}
    quint32 startOrder() const { return _startOrder;}
    quint32 startLine() const { return _startLine;}
    quint32 endOrder() const { return _endOrder;}
    quint32 endLine() const { return _endLine;}
    QRgb color() const { return _color;}

private:
    quint32 _startOrder;
    quint32 _startLine;
    quint32 _endOrder;
    quint32 _endLine;
    QRgb _color;
};

class AssemblyDefinition : public QObject
{
    Q_OBJECT
public:
    explicit AssemblyDefinition(QObject *parent = 0);

    // Getters/setters
    QString version() const;
    void setVersion(const QString &version);

    QList<ProcessorDefinition*> processorDefs() const;
    QList<ConnectionDefinition*> connectionDefs() const;

    QString date() const;
    void setDate(const QString &date);

    QString author() const;
    void setAuthor(const QString &author);

    QString comment() const;
    void setComment(const QString &comment);

    // Others
    void addProcessorDef(ProcessorDefinition *processorDef);
    void addConnectionDef(ConnectionDefinition *connectionDef);

    SourceDefinition* sourceDefinition() const;
    void setSourceDefinition(SourceDefinition *sourceDefinition);

    ParameterDefinition* parametersDefinition() const;
    void setParametersDefinition(ParameterDefinition *parametersDefinition);

    DestinationDefinition* destinationDefinition() const;
    void setDestinationDefinition(DestinationDefinition *destinationDefinition);

    QString filename() const;
    void setFilename(const QString &filename);

    QString name() const;
    void setName(const QString &name);

    bool usable() const;
    void setUsable(bool usable);

    QDate creationDate() const;
    void setCreationDate(const QDate &creationDate);

    bool isRealTime() const;
    void setIsRealTime(bool isRealTime);

    QString serialized();

    QList<AssemblyDefinitionValidity> checkDefinition();


signals:

public slots:

private:
    QString _filename;
    QString _name;
    QString _version;
    QString _date;
    QString _author;
    QString _comment;
    QDate _creationDate;
    bool _usable;
    bool _isRealTime;

    SourceDefinition *_sourceDefinition;
    ParameterDefinition *_parametersDefinition;
    QList<ProcessorDefinition*> _processorDefs;
    QList<ConnectionDefinition*> _connectionDefs;
    DestinationDefinition *_destinationDefinition;

};
}

#endif // ASSEMBLY_H
#ifndef MATISSE_ASSEMBLY_DEFINITION_H_
#define MATISSE_ASSEMBLY_DEFINITION_H_

#include <QObject>
#include <QtGlobal>
#include <QList>
#include <QDate>
#include <QRgb>
#include <QMultiHash>
#include <QStringList>

///
/// Business object for assemblies
///

namespace matisse {

enum eAssemblyDefinitionValidity {
    VALID = 0,
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
    SourceDefinition(QString _name) :
        m_id(0),
        m_name(_name),
        m_order(0){}

    quint32 id() const { return m_id;}
    QString name() const { return m_name;}
    quint32 order() const { return m_order;}

private:
    quint32 m_id;
    QString m_name;
    quint32 m_order;
};

class DestinationDefinition
{
    friend class AssemblyDefinition;
public:
    DestinationDefinition(QString _name, quint32 _order) :
        m_id(0),
        m_name(_name),
        m_order(_order){}

    quint32 id() const { return m_id;}
    QString name() const { return m_name;}
    quint32 order() const { return m_order;}

private:
    quint32 m_id;
    QString m_name;
    quint32 m_order;
};

class ProcessorDefinition {
    friend class AssemblyDefinition;
public:
    ProcessorDefinition(QString _name, quint32 _order):
        m_id(0),
        m_name(_name),
        m_order(_order) {}

    quint32 id() const { return m_id;}
    QString name() const { return m_name;}
    quint32 order() const { return m_order;}

private:
    quint32 m_id;
    QString m_name;
    quint32 m_order;
};

class ConnectionDefinition {
    friend class AssemblyDefinition;
public:
    ConnectionDefinition(quint32 _start_order, quint32 _start_line, quint32 _end_order, quint32 _end_line, QRgb _color) :
        m_start_order(_start_order),
        m_start_line(_start_line),
        m_end_order(_end_order),
        m_end_line(_end_line),
        m_color(_color){}
    quint32 startOrder() const { return m_start_order;}
    quint32 startLine() const { return m_start_line;}
    quint32 endOrder() const { return m_end_order;}
    quint32 endLine() const { return m_end_line;}
    QRgb color() const { return m_color;}

private:
    quint32 m_start_order;
    quint32 m_start_line;
    quint32 m_end_order;
    quint32 m_end_line;
    QRgb m_color;
};

class AssemblyDefinition : public QObject
{
    Q_OBJECT
public:
    explicit AssemblyDefinition(QObject *_parent = 0);

    // Getters/setters
    QString version() const;
    void setVersion(const QString &_version);

    QList<ProcessorDefinition*> processorDefs() const;
    QList<ConnectionDefinition*> connectionDefs() const;

    QString date() const;
    void setDate(const QString &_date);

    QString author() const;
    void setAuthor(const QString &_author);

    QString comment() const;
    void setComment(const QString &_comment);

    // Others
    void addProcessorDef(ProcessorDefinition *_processor_def);
    void addConnectionDef(ConnectionDefinition *_connection_def);

    SourceDefinition* sourceDefinition() const;
    void setSourceDefinition(SourceDefinition *_source_definition);

    DestinationDefinition* destinationDefinition() const;
    void setDestinationDefinition(DestinationDefinition *_destination_definition);

    QString filename() const;
    void setFilename(const QString &_filename);

    QString name() const;
    void setName(const QString &_name);

    bool usable() const;
    void setUsable(bool _usable);

    QDate creationDate() const;
    void setCreationDate(const QDate &_creation_date);

    QList<eAssemblyDefinitionValidity> checkDefinition();

    AssemblyDefinition * duplicate(QString _new_name, QString _new_file_name);
    void clearAllElements();

signals:

public slots:

private:
    QString m_filename;
    QString m_name;
    QString m_version;
    QString m_date;
    QString m_author;
    QString m_comment;
    QDate m_creation_date;
    bool m_usable;

    SourceDefinition *m_source_definition;
    QList<ProcessorDefinition*> m_processor_defs;
    QList<ConnectionDefinition*> m_connection_defs;
    DestinationDefinition *m_destination_definition;

};

} // namespace matisse

#endif // MATISSE_ASSEMBLY_DEFINITION_H_

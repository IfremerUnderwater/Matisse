#ifndef PLUGININTERFACE_H
#define PLUGININTERFACE_H

#include <QList>
#include <QVariant>
#include <QString>
#include <QThread>
#include <QtDebug>

enum ParameterType {
    INT,
    REAL,
    STRING,
    BOOL
};

enum ParameterShow {
    LINE_EDIT,
    COMBO_BOX,
    SPIN_BOX,
    SLIDER,
    FILE_SELECTOR,
    DIR_SELECTOR
};

struct ProcessorParameter {
    QString _name;
    QString _comments;
    QString _prefix;
    QString _suffix;
    ParameterType _type;
    ParameterShow _show;
    QVariant _value;
    QVariant _minValue;
    QVariant _maxValue;
    bool _required;
};

class PluginInterface: public QThread
{ 
public:

    virtual ~PluginInterface(){}
    virtual QString processorId(){return _processorId;}
    virtual QString processorName(){return _processorName;}
    virtual QString processorComments(){return _comments;}
    virtual int errorCode(){return _errorCode;}
    virtual QList<ProcessorParameter> parameters(){return _parameters;}

    virtual PluginInterface * newInstance() = 0;

    void runProcessor(const QList<QVariant> &args) {_args = args;
                                                    qDebug() << "Run processor";
                                                    start();
                                                   }
protected:
    virtual void init() = 0;
    virtual void run() = 0;

    QList<QVariant> _args;
    QString _processorId;
    QString _processorName;
    QString _comments;
    int _errorCode;
    QList<ProcessorParameter> _parameters;
};

Q_DECLARE_INTERFACE(PluginInterface, "TestTCA.PluginInterface/1.0")

#endif // PLUGININTERFACE_H

#ifndef MATISSEPARAMETERSMANAGER_H
#define MATISSEPARAMETERSMANAGER_H

#include <QObject>
#include <QDialog>
#include <QtXml>
#include <QtXmlPatterns>
#include <QtDebug>
#include <QFile>
#include <QMap>
#include <QHash>
#include <QStringRef>
#include <QScriptEngine>
#include <QScriptValue>
#include <QSize>
#include <QRegExp>
#include <QStringList>
#include "ToolsCommon.h"
#include "EnrichedFormWidget.h"
#include "ParametersWidgetSkeleton.h"
#include "MatisseParameters.h"
#include "KeyValueList.h"
#include "EnrichedComboBox.h"
#include "EnrichedLineEdit.h"
#include "EnrichedListBox.h"
#include "EnrichedSpinBox.h"
#include "EnrichedDoubleSpinBox.h"
#include "EnrichedCheckBox.h"
#include "EnrichedTableWidget.h"
#include "EnrichedFileChooser.h"

namespace MatisseTools {

class MatisseParametersManager : public QObject
{
    Q_OBJECT
public:
    explicit MatisseParametersManager(QObject *parent = 0);

    bool readDictionnaryFile(QString xmlFilename);
    bool addUserModuleForParameter(QString userModule, QString structureName, QString paramName);
    bool removeUserModuleForParameter(QString userModule, QString structureName, QString paramName);
    bool clearExpectedParameters();

    bool addParameter(QString structName, QString groupName, QString groupText, QXmlStreamAttributes attributes);
    bool addEnum(QString enumsName, QXmlStreamAttributes attributes);
//    ParametersWidgetSkeleton * generateParametersWidget(QWidget *owner);
    ParametersWidgetSkeleton *generateParametersWidget(QWidget *owner);
    ParametersWidgetSkeleton * parametersWidget() { return _fullParametersWidget; }

    bool saveParametersValues(QString entityName, bool isAssemblyTemplate);
    bool loadParameters(QString entityName, bool isAssemblyTemplate);
    void restoreParametersDefaultValues();

    void createJobParametersFile(QString assemblyName, QString jobName, KeyValueList kvl);

    void checkDictionnaryComplete();
    void updateDatasetParametersValues(KeyValueList kvl);
signals:

public slots:

private:
    void loadStaticCollections();
//    ParametersWidgetSkeleton *createDialog(QWidget *owner, QString structName, bool user);
    bool getRange(Parameter param, QVariant &minValue, QVariant &maxValue);
    QString getValue(QString structName, QString parameterName);
    qint32 getIntValue(QVariant value);
    bool getBoolValue(QVariant value);
    qreal getDoubleValue(QVariant value);
    QStringList getNumList(Parameter param);
    QStringList getEnums(Parameter param);
    //bool generateParametersFile(QString filename);
    bool readParametersFile(QString filename, bool isAssemblyTemplate);
    bool writeParametersFile(QString parametersFilename, bool overwrite=false);
    void generateLevelParametersWidget(ParameterLevel level, QString levelHeader);

    QMap<QString, Structure> _structures;
    QMap<QString, ParametersGroup> _groups;
    QMap<QString, Parameter> _parameters;
    QStringList _structuresNames;
    QMap<QString, Enums> _enums;
    QMap<QString, QString> _structureByParameter;
    QMap<QString, QSet<QString>*> _expectedParameters;
    QMap<QString, QString> _groupByParameter;
    QMap<ParameterLevel, QList<QString>*> _parametersByLevel;
    QMap<QString, QSet<QString>*> _expectedGroups;
    QSet<QString> _jobExtraParameters;
    QDateTime _dicoPublicationTimestamp;
    QString _selectedAssembly;

    ParametersWidgetSkeleton * _fullParametersWidget;
    QMap<QString, ParametersWidgetSkeleton*> _dialogs;
    QMap<QString, QMap<QString, EnrichedFormWidget*> > _valuesWidgets;
    QMap<QString, QMap<QString, QWidget*> > _groupsWidgets;

    QMap<ParameterLevel, QPushButton*> _headerByLevel;
    QMap<ParameterLevel, QWidget*> _ParamContainerByLevel;

    static QMap<QString, ParameterType> _enumTypes;
    static QMap<QString, ParameterLevel> _enumLevels;
    static QMap<QString, ParameterShow> _enumShows;

    static QSet<QString> _datasetParamNames;

    static QRegExp _intervalRangeExpr;
    static QRegExp _setRangeExpr;
    static QRegExp _matrixExpr;
    static QRegExp _matrixValuesExpr;

    static QString _infStr;
    static double _epsilon;
};

}

#endif // MATISSEPARAMETERSMANAGER_H

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
#include <QSize>
#include <QRegExp>
#include <QStringList>
#include "ToolsCommon.h"
#include "EnrichedFormWidget.h"
#include "ParametersWidgetSkeleton.h"
#include "MatisseParameters.h"
#include "KeyValueList.h"
#include "EnrichedComboBox.h"
#include "EnrichedCamComboBox.h"
#include "EnrichedLineEdit.h"
#include "EnrichedListBox.h"
#include "EnrichedSpinBox.h"
#include "EnrichedDoubleSpinBox.h"
#include "EnrichedCheckBox.h"
#include "EnrichedTableWidget.h"
#include "EnrichedFileChooser.h"
#include "MatisseDictionnaryLabels.h"
#include "ParametersHeaderButton.h"
#include "MatisseIconFactory.h"

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
    ParametersWidgetSkeleton *generateParametersWidget(QWidget *owner);
    ParametersWidgetSkeleton * parametersWidget() { return _fullParametersWidget; }

    bool saveParametersValues(QString entityName, bool isAssemblyTemplate);
    bool loadParameters(QString entityName, bool isAssemblyTemplate);
    void restoreParametersDefaultValues();

    void createJobParametersFile(QString assemblyName, QString jobName, KeyValueList kvl);

    void applyApplicationContext(bool isExpert, bool isProgramming);
    void toggleReadOnlyMode(bool isReadOnly);
    void pullDatasetParameters(KeyValueList &kvl);
    void pushDatasetParameters(KeyValueList kvl);
    void pushPreferredDatasetParameters(KeyValueList kvl);

    QString getParameterValue(QString parameterName);

    void setIconFactory(MatisseIconFactory *iconFactory);

signals:

protected slots:
    void slot_translateParameters();
    void slot_foldUnfoldLevelParameters();

private:
    void loadStaticCollections();
    bool getRange(Parameter param, QVariant &minValue, QVariant &maxValue);
    QString getValue(QString structName, QString parameterName);
    qint32 getIntValue(QVariant value);
    bool getBoolValue(QVariant value);
    double getDoubleValue(QVariant value);
    QStringList getNumList(Parameter param);
    QStringList getEnums(Parameter param);
    bool readParametersFile(QString filename, bool isAssemblyTemplate);
    bool writeParametersFile(QString parametersFilename, bool overwrite=false);
    void generateLevelParametersWidget(ParameterLevel level);
    void translateHeaderButtons();
    void retranslateLevelGroups(ParameterLevel level);
    void checkDictionnaryComplete();

    MatisseDictionnaryLabels _dictionnaryLabels;

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
    QMap<QString, QString> _preferredDatasetParameters;
    QDateTime _dicoPublicationTimestamp;
    QString _selectedAssembly;

    ParametersWidgetSkeleton * _fullParametersWidget;
    QMap<QString, QMap<QString, QWidget*> > _groupsWidgets;
    QMap<QString, QMap<QString, EnrichedFormWidget*> > _valuesWidgets;
    QMap<QString, ParametersWidgetSkeleton*> _dialogs;

    QMap<QString, EnrichedFormWidget*> _valueWidgetsByParamName;
    QMap<ParameterLevel, QMap<QString, QGroupBox*>* > _groupWidgetsByLevel;

    QMap<ParameterLevel, ParametersHeaderButton*> _headerButtonsByLevel;
    QMap<ParameterLevel, QWidget*> _paramContainersByLevel;

    bool _isReadOnlyMode;

    MatisseIconFactory *_iconFactory;

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

#include "MatisseParametersManager.h"

using namespace MatisseCommon;
using namespace MatisseTools;

QMap<QString, ParameterType> MatisseParametersManager::_enumTypes;
QMap<QString, ParameterLevel> MatisseParametersManager::_enumLevels;
QMap<QString, ParameterShow> MatisseParametersManager::_enumShows;

QSet<QString> MatisseParametersManager::_datasetParamNames;

QRegExp MatisseParametersManager::_intervalRangeExpr("^(\\[|\\])((\\-?\\d+((\\.)\\d+)?)|(-inf)),((\\-?\\d+((\\.)\\d+)?)|(\\+?inf))(\\[|\\])$");
QRegExp MatisseParametersManager::_setRangeExpr("^\\{\\w+(,\\w+)*\\}$");

QRegExp MatisseParametersManager::_matrixExpr("^\\(\\d+(,\\d+)?\\)$");
QRegExp MatisseParametersManager::_matrixValuesExpr("^\\-?\\d+((\\.)\\d+)?(;\\-?\\d+((\\.)\\d+)?)*$");

double MatisseParametersManager::_epsilon = 0.000001;
QString MatisseParametersManager::_infStr = QString::number(quint64(InfInt));


MatisseParametersManager::MatisseParametersManager(QObject *parent) :
    QObject(parent),
    _structures(),
    _dialogs(),
    _groupsWidgets(),
    _valuesWidgets(),
    _selectedAssembly(""),
    _dictionnaryLabels(),
    _isReadOnlyMode(false)
{
    loadStaticCollections();
    _dictionnaryLabels.declareLabels();
}

void MatisseParametersManager::loadStaticCollections()
{
    if (!_enumTypes.isEmpty()) {
        // static collections already initialized
        return;
    }

    //_enumTypes.clear();
    //_enumTypes.insert("unknown", UNKNOWN_TYPE);
    _enumTypes.insert("integer", PAR_INT);
    _enumTypes.insert("float", PAR_FLOAT);
    _enumTypes.insert("double", PAR_DOUBLE);
    _enumTypes.insert("string", PAR_STRING);
    _enumTypes.insert("bool", PAR_BOOL);

    //_enumShows.clear();
    //_enumShows.insert("unknown", UNKNOWN_SHOW);
    _enumShows.insert("line", LINE_EDIT);
    _enumShows.insert("combo", COMBO_BOX);
    _enumShows.insert("list", LIST_BOX);
    _enumShows.insert("spin", SPIN_BOX);
    _enumShows.insert("spinDouble", DOUBLE_SPIN_BOX);
    _enumShows.insert("check", CHECK_BOX);
    _enumShows.insert("table", TABLE);
    _enumShows.insert("file", FILE_SELECTOR_ABSOLUTE);
//    _enumShows.insert("file:relative", FILE_SELECTOR_RELATIVE);
//    _enumShows.insert("file:absolute", FILE_SELECTOR_ABSOLUTE);
    _enumShows.insert("dir", DIR_SELECTOR_ABSOLUTE);
//    _enumShows.insert("dir:relative", DIR_SELECTOR_RELATIVE);
//    _enumShows.insert("dir:absolute", DIR_SELECTOR_ABSOLUTE);

    _enumLevels.insert("user", USER);
    _enumLevels.insert("advanced", ADVANCED);
    _enumLevels.insert("expert", EXPERT);

    _datasetParamNames.insert(DATASET_PARAM_OUTPUT_DIR);
    _datasetParamNames.insert(DATASET_PARAM_OUTPUT_FILENAME);
    _datasetParamNames.insert(DATASET_PARAM_DATASET_DIR);
    _datasetParamNames.insert(DATASET_PARAM_NAVIGATION_FILE);
}

void MatisseParametersManager::checkDictionnaryComplete()
{
    if (!_structures.contains(DATASET_STRUCTURE)) {
        qFatal("Dictionnary does not contain mandatory 'dataset_param' structure");
    }

    Structure datasetStruct = _structures.value(DATASET_STRUCTURE);
    QList<ParametersGroup> datasetGroups = datasetStruct._parametersGroups;

    QSet<QString> foundDatasetParams;

    foreach (ParametersGroup group, datasetGroups) {
        QStringList params = group._parametersNames;

        foreach (QString param, params) {
            if (_datasetParamNames.contains(param)) {
                foundDatasetParams.insert(param);
            }
        }
    }

    if (foundDatasetParams != _datasetParamNames) {

        QStringList found = foundDatasetParams.values();
        QStringList expected = _datasetParamNames.values();
        QString fullMessage = QString("Dataset parameters were not all found in the dictionnary\Expected: ")
                .append(expected.join(",")).append("\nFound: ").append(found.join(","));

        qFatal(fullMessage.toLatin1());
    }
}

bool MatisseParametersManager::readDictionnaryFile(QString xmlFilename)
{
    _structures.clear();

    QFile inputFile(xmlFilename);
    if (!inputFile.exists()) {
        qDebug() << "Fichier non trouvé..." << xmlFilename;
        return false;
    }

    if (!inputFile.open(QIODevice::ReadOnly)) {
        qDebug() << "Fichier non ouvert...";
        return false;
    }

    QXmlStreamReader reader(&inputFile);
    QXmlStreamAttributes attributes;

    QString structureName;
    QString groupName;
    QString groupText = "";
    QString enumsName;
    while (!reader.atEnd()) {
        /* Read next element.*/
        QXmlStreamReader::TokenType token = reader.readNext();
        /* If token is just StartDocument, we'll go to next.*/
        if(token == QXmlStreamReader::StartDocument) {
           continue;
        }

        /* If token is StartElement, we'll see if we can read it.*/
        if(token == QXmlStreamReader::StartElement) {

            QString name = reader.name().toString();
            attributes = reader.attributes();

            if (name == "Structures") {
                _dicoPublicationTimestamp = QDateTime::fromString(attributes.value("publicationTimestamp").toString(), "yyyy-MM-dd'T'hh:mm:ss");
            } else if (name == "Structure") {
                structureName = attributes.value("name" ).toString();
                // qDebug() << "Trouvé structure" << structureName;
                Structure newStruct;
                newStruct._name = structureName;
                newStruct._hasUserValues = false;

                /* check structure name unicity (not supported by Qt XML schema validation) */
                if (_structures.contains(structureName)) {
                    qFatal(QString("Unique constraint violation for structure name '%1'").arg(structureName).toLatin1());
                }
                _structures.insert(structureName, newStruct);
                _structuresNames << structureName;
            } else if (name == "ParametersGroup") {
                groupName = attributes.value("name").toString();
                groupText = tr(attributes.value("text").toString().toLatin1());
            } else if (name == "Parameter") {
                // qDebug() << "Add Parameter:" << structureName << groupName << attributes.value("name");
                addParameter(structureName, groupName, groupText, attributes);
            } else if (name == "Enum") {
                enumsName = attributes.value("name" ).toString();
                // qDebug() << "Trouvé enum" << enumsName;
                Enums newEnums;
                newEnums._name = enumsName;
                _enums.insert(enumsName, newEnums);
            } else if (name == "EnumValue") {
                addEnum(enumsName, attributes);
            }
        }

        if (reader.hasError()) {
            qWarning() << "An error occurred while reading the dictionnary file:\n" << reader.errorString();
        }
    }

    reader.clear();
    inputFile.close();

    // check dictionnary integrity
    checkDictionnaryComplete();

    return (_structures.size() > 0);
}

bool MatisseParametersManager::addUserModuleForParameter(QString userModule, QString structureName, QString paramName)
{
    if (!_structures.contains(structureName)) {
        qCritical() << "Structure not found in parameters dictionnary : " << structureName;
        return false;
    }

    if (!_structureByParameter.contains(paramName)) {
        qCritical() << "Parameter not found in dictionnary : " << paramName;
        return false;
    }

    QString dicoStructName = _structureByParameter.value(paramName);
    if (dicoStructName != structureName) {
        qWarning() << QString("Parameter '%1' found in dictionnary in structure '%2', not '%3'").arg(paramName).arg(dicoStructName).arg(structureName);
    }

    if (!_expectedParameters.contains(paramName)) {
        qDebug() << "Adding expected parameter " << dicoStructName << paramName;
        QSet<QString> *paramUsers = new QSet<QString>();
        paramUsers->insert(userModule);
        _expectedParameters.insert(paramName, paramUsers);
    } else {
        qDebug() << "Adding user" << userModule << "for param" << dicoStructName << paramName;
        QSet<QString> *paramUsers = _expectedParameters.value(paramName);
        if (paramUsers->contains(userModule)) {
            qWarning() << QString("Module '%1' is already referenced as using param").arg(userModule) << dicoStructName << paramName;
        } else {
            paramUsers->insert(userModule);
        }
    }

    // Not necessary to check the presence of the key (already checked for the structure)
    QString groupName = _groupByParameter.value(paramName);

    if (!_expectedGroups.contains(groupName)) {
        qDebug() << "Adding expected group" << groupName;
        QSet<QString> *groupMembers = new QSet<QString>();
        groupMembers->insert(paramName);
        _expectedGroups.insert(groupName, groupMembers);
    } else {
        qDebug() << "Adding param" << paramName << "as member of expected group" << groupName;
        QSet<QString> *groupMembers = _expectedGroups.value(groupName);
        if (groupMembers->contains(paramName)) {
            qDebug() << QString("Param '%1' is already referenced as member of expected group '%2'").arg(paramName).arg(groupName);
        } else {
            groupMembers->insert(paramName);
        }
    }

    if (!_valuesWidgets.contains(dicoStructName)) {
        qCritical() << "Widgets hash not found for structure " << dicoStructName;
        return false;
    }

    QMap<QString, EnrichedFormWidget*> structureParamsWidgets = _valuesWidgets.value(dicoStructName);
    QMap<QString, QWidget*> structureGroupsWidgets = _groupsWidgets.value(dicoStructName);

    if (!structureParamsWidgets.contains(paramName)) {
        qCritical() << QString("Widgets hash for structure '%1' does not contain widget for parameter '%2'").arg(dicoStructName).arg(paramName);
        return false;
    }

    EnrichedFormWidget* paramWidget = structureParamsWidgets.value(paramName);
    QWidget* groupWidget = structureGroupsWidgets.value(paramName);
    if (!paramWidget) {
        qCritical() << "Widgets for parameter" << paramName << "is null";
        return false;
    }

    groupWidget->show();
    paramWidget->show();

    return true;
}

bool MatisseParametersManager::removeUserModuleForParameter(QString userModule, QString structureName, QString paramName)
{
    if (!_structures.contains(structureName)) {
        qCritical() << "Structure not found in parameters dictionnary : " << structureName;
        return false;
    }

    if (!_structureByParameter.contains(paramName)) {
        qCritical() << "Parameter not found in dictionnary : " << paramName;
        return false;
    }

    QString dicoStructName = _structureByParameter.value(paramName);
    if (dicoStructName != structureName) {
        qWarning() << QString("Parameter '%1' found in dictionnary in structure '%2', not '%3'").arg(paramName).arg(dicoStructName).arg(structureName);
    }

    if (!_expectedParameters.contains(paramName)) {
        qCritical() << QString("Parameter '%1' not referenced as expected parameter");
        return false;
    }

    QSet<QString> *users = _expectedParameters.value(paramName);
    if (!users->contains(userModule)) {
        qCritical() << QString("User module '%1' not referenced as expecting parameter '%2'").arg(userModule).arg(paramName);
        return false;
    }

    if (!_groupByParameter.contains(paramName)) {
        qCritical() << QString("No group found for parameter '%1'").arg(paramName);
        return false;
    }

    QString groupName = _groupByParameter.value(paramName);

    if (!_expectedGroups.contains(groupName)) {
        qCritical() << QString("No members list found for group '%1'").arg(groupName);
        return false;
    }

    QSet<QString> *groupMembers = _expectedGroups.value(groupName);
    if (!groupMembers->contains(paramName)) {
        qCritical() << QString("Parameter '%1' not found in members list for group '%2'").arg(paramName).arg(groupName);
        return false;
    }

    qDebug() << QString("Removing module '%1' as expecting parameter '%2'").arg(userModule).arg(paramName);
    users->remove(userModule);

    if (users->isEmpty()) {
        qDebug() << QString("Parameter '%1' is no longer expected, hiding field").arg(paramName);
        _expectedParameters.remove(paramName);
        delete users;

        // de-reference parameter for group members
        groupMembers->remove(paramName);

        bool hideGroup = false;

        // de-reference group if does not contain any expected parameter
        if (groupMembers->isEmpty()) {
            _expectedGroups.remove(groupName);
            delete groupMembers;
            hideGroup = true;
        } else {
            // see if group still has other parameters of same level
            hideGroup = true;

            Parameter parameter = _parameters.value(paramName);

            foreach (QString otherParamName, *groupMembers) {
                Parameter otherParameter = _parameters.value(otherParamName);
                if (otherParameter._level == parameter._level) {
                    hideGroup = false;
                    break;
                }
            }
        }

        /* handling parameters and groups hiding if necessary */
        QMap<QString, EnrichedFormWidget*> structureParamsWidgets = _valuesWidgets.value(dicoStructName);
        QMap<QString, QWidget*> structureGroupsWidgets = _groupsWidgets.value(dicoStructName);

        if (!structureParamsWidgets.contains(paramName)) {
            qCritical() << QString("Widgets hash for structure '%1' does not contain widget for parameter '%2'").arg(dicoStructName).arg(paramName);
            return false;
        }

        EnrichedFormWidget* paramWidget = structureParamsWidgets.value(paramName);
        QWidget* groupWidget = structureGroupsWidgets.value(paramName);
        if (!paramWidget) {
            qCritical() << "Widgets for parameter" << paramName << "is null";
            return false;
        }

        paramWidget->hide();
        if (hideGroup) {
            groupWidget->hide();
        }
    }

    return true;
}

bool MatisseParametersManager::clearExpectedParameters()
{
    QSet<QWidget*> groupWidgets;

    // clear expected parameters and using modules
    QList<QString> params = _expectedParameters.keys();
    foreach (QString param, params) {
        QSet<QString> *users = _expectedParameters.value(param);
        _expectedParameters.remove(param);
        users->clear();
        delete users;

        QString structure = _structureByParameter.value(param);
        QWidget* paramWidget = _valuesWidgets.value(structure).value(param);
        paramWidget->hide();

        QWidget* groupWidget = _groupsWidgets.value(structure).value(param);
        if (!groupWidgets.contains(groupWidget)) {
            groupWidgets.insert(groupWidget);
        }
    }

    // clear expected groups and member parameters
    QList<QString> groups = _expectedGroups.keys();
    foreach (QString group, groups) {
        QSet<QString> *members = _expectedGroups.value(group);
        _expectedGroups.remove(group);
        members->clear();
        delete members;
    }

    foreach (QWidget* groupWidget, groupWidgets) {
        groupWidget->hide();
    }
    groupWidgets.clear();

    return true;
}

bool MatisseParametersManager::addParameter(QString structName, QString groupName, QString groupText, QXmlStreamAttributes attributes)
{
    if (!_structures.contains(structName)) {
        qCritical() << QString("Structure '%1'' not found, cannot add parameter").arg(structName);
        return false;
    }

    // recherche du groupe s'il n'existe pas, on le crée. Si pas de nom de groupe
    int noGroup = _structures[structName]._groupsNames.indexOf(groupName);
    if (noGroup == -1) {
        _structures[structName]._groupsNames.append(groupName);
        ParametersGroup newGroup;
        newGroup._name = groupName;
        newGroup._text = groupText;
        newGroup._hasUserValues = false;
        _structures[structName]._parametersGroups.append(newGroup);
        noGroup = _structures[structName]._groupsNames.size() - 1;

        /* check unique constraint for group name */
        if (_groups.contains(groupName)) {
            qFatal(QString("Unique constraint violation for group name '%1'").arg(groupName).toLatin1());
        }

        _groups.insert(groupName, newGroup);
    }

    Parameter parameter;

    parameter._name = attributes.value("name").toString();

    QString levelStr = attributes.value("level").toString().toLower();
    ParameterLevel level = _enumLevels.value(levelStr);
    parameter._level = level;

    parameter._text = tr(attributes.value("text").toLatin1());

    QString typeStr = attributes.value("type").toString().toLower();
    parameter._type = _enumTypes.value(typeStr);

    QString showStr = attributes.value("show").toString().toLower();
    parameter._show = _enumShows.value(showStr);

    // double spin...
    if (((parameter._type == PAR_DOUBLE) || (parameter._type == PAR_FLOAT)) && (parameter._show == SPIN_BOX)) {
        parameter._show = _enumShows.value("spinDouble");
    }

    parameter._parameterSize = QSize(1,1);
    QString sizeStr = attributes.value("size").toString().simplified().replace(" ","");
    if (_matrixExpr.exactMatch(sizeStr)) {
        sizeStr = sizeStr.mid(1, sizeStr.size()-2);
        parameter._parameterSize.setWidth(sizeStr.split(",").at(0).toInt());
        parameter._parameterSize.setHeight(sizeStr.split(",").at(1).toInt());
    }

    quint8 precisionDefault = PRECISION_DEFAULT;

    parameter._precision = precisionDefault; /* default is 2 decimal digits */
    if (attributes.hasAttribute("precision")) {
        quint8 precision = QString(attributes.value("precision").toLatin1()).toUShort();
        quint8 precisionMax = PRECISION_MAX;
        if (precision > precisionMax) {
            qWarning() << QString("Parameter '%1' : precision %2 exceeds max precision %3. Leaving default precision %4'").arg(parameter._name).arg(precision).arg(precisionMax).arg(precisionDefault);
        } else {
            parameter._precision = precision;
        }
    }

    if (attributes.hasAttribute("formatTemplate")) {
        parameter._formatTemplate = attributes.value("formatTemplate").toString();
    } else {
        parameter._formatTemplate = "";
    }

    parameter._range = attributes.value("range").toString().simplified().replace(" ","");

    parameter._value = attributes.value("default").toString();

    QString paramName = parameter._name;

    _structures[structName]._parametersGroups[noGroup]._parametersNames << paramName;
    _structures[structName]._parametersGroups[noGroup]._parameters.append(parameter);

    /* check unique constraint for parameter name */
    if (_parameters.contains(paramName)) {
        qFatal(QString("Unique constraint violation for parameter name '%1'").arg(paramName).toLatin1());
    }
    _parameters.insert(paramName, parameter);

    _structureByParameter.insert(paramName, structName);

    _groupByParameter.insert(paramName, groupName);

    /* Appending parameter name to the usage level list */
    QList<QString>* paramsForLevel;

    if (!_parametersByLevel.contains(level)) {
        paramsForLevel = new QList<QString>;
        _parametersByLevel.insert(level, paramsForLevel);
    } else {
        paramsForLevel = _parametersByLevel.value(level);
    }

    paramsForLevel->append(paramName);

    return true;

}

bool MatisseParametersManager::addEnum(QString enumsName, QXmlStreamAttributes attributes)
{
    if (!_enums.contains(enumsName)) {
        return false;
    }
    EnumValue enumValue;

    enumValue._name = attributes.value("name").toString();
    enumValue._text = attributes.value("text").toString();

    _enums[enumsName]._values.append(enumValue);

    return true;

}

ParametersWidgetSkeleton *MatisseParametersManager::generateParametersWidget(QWidget *owner)
{
    qDebug() << "Building parameters widget";
    _fullParametersWidget = new ParametersWidgetSkeleton(owner);
    _fullParametersWidget->setObjectName("_WID_parametersWidget");

    generateLevelParametersWidget(USER);
    generateLevelParametersWidget(ADVANCED);
    generateLevelParametersWidget(EXPERT);

    translateHeaderButtons();

    connect(_fullParametersWidget, SIGNAL(signal_translateParameters()), this, SLOT(slot_translateParameters()));

    applyApplicationContext(false, false);

    return _fullParametersWidget;
}

void MatisseParametersManager::generateLevelParametersWidget(ParameterLevel level)
{
    ParametersHeaderButton *levelHeaderButton = new ParametersHeaderButton(_fullParametersWidget, level);
    levelHeaderButton->setObjectName("_PB_levelHeaderButton");
    _fullParametersWidget->addWidget(levelHeaderButton);

    _headerButtonsByLevel.insert(level, levelHeaderButton);

    connect(levelHeaderButton, SIGNAL(clicked(bool)), this, SLOT(slot_foldUnfoldLevelParameters()));

    QWidget *levelContainer = new QWidget(_fullParametersWidget);
    levelContainer->setObjectName("_WID_levelParametersContainer");
    QVBoxLayout *levelContainerLayout = new QVBoxLayout();
    levelContainer->setLayout(levelContainerLayout);
    _fullParametersWidget->addWidget(levelContainer);

    _paramContainersByLevel.insert(level, levelContainer);

    QList<QString>* levelParamsList = _parametersByLevel.value(level);

    QSet<QString> levelGroups;

    QGroupBox* currentGroup = NULL;

    foreach (QString paramName, *levelParamsList) {
        QString structName = _structureByParameter.value(paramName);
        QString groupName = _groupByParameter.value(paramName);

        ParametersGroup group;
        QList<ParametersGroup> groups = _structures[structName]._parametersGroups;
        foreach (ParametersGroup g, groups) {
            if (g._name == groupName) {
                group = g;
                break;
            }
        }

        if (!levelGroups.contains(groupName)) {
            /* Hide previous group */
            if (currentGroup) {
                currentGroup->hide();
            }

            QString groupLabel = _dictionnaryLabels.getGroupLabel(groupName);
            //currentGroup = new QGroupBox(group._text, _fullParametersWidget);
            currentGroup = new QGroupBox(groupLabel, _fullParametersWidget);
            QVBoxLayout* currentGroupLayout = new QVBoxLayout();
            currentGroupLayout->setContentsMargins(0, PARAM_GROUP_MARGIN_TOP, 0, PARAM_GROUP_MARGIN_BOTTOM);

            currentGroup->setLayout(currentGroupLayout);
            levelContainerLayout->addWidget(currentGroup);

            levelGroups.insert(groupName);

            /* referencing group for deferred translation */
            QMap<QString, QGroupBox*> *levelGroupWidgets;
            if (!_groupWidgetsByLevel.contains(level)) {
                levelGroupWidgets = new QMap<QString, QGroupBox*>();
                _groupWidgetsByLevel.insert(level, levelGroupWidgets);
            } else {
                levelGroupWidgets = _groupWidgetsByLevel.value(level);
            }

            levelGroupWidgets->insert(groupName, currentGroup);
        }

        Parameter param;
        QList<Parameter> params = group._parameters;
        foreach (Parameter p, params) {
            if (p._name == paramName) {
                param = p;
                break;
            }
        }

        EnrichedFormWidget * widget = NULL;
        QString paramLabelText = _dictionnaryLabels.getParamLabel(paramName);

        switch(param._show) {
        case LINE_EDIT: {
            widget = new EnrichedLineEdit(_fullParametersWidget, paramLabelText, param._value.toString());
         }
            break;
        case SPIN_BOX: {
            QVariant minValue;
            QVariant maxValue;
            getRange(param, minValue, maxValue);
            widget = new EnrichedSpinBox(_fullParametersWidget, paramLabelText, minValue.toString(), maxValue.toString(), param._value.toString());
        }
            break;
        case DOUBLE_SPIN_BOX: {
            QVariant minValue;
            QVariant maxValue;
            getRange(param, minValue, maxValue);
            EnrichedDecimalValueWidget* decimalValueWidget = new EnrichedDoubleSpinBox(_fullParametersWidget, paramLabelText, minValue.toString(), maxValue.toString(), param._value.toString());
            decimalValueWidget->setPrecision(param._precision);
            widget = decimalValueWidget;
        }
            break;
        case COMBO_BOX: {
            QStringList items = getEnums(param);
            widget = new EnrichedComboBox(_fullParametersWidget, paramLabelText, items, param._value.toString());
        }
            break;
        case LIST_BOX: {
            QStringList items = getEnums(param);
            widget = new EnrichedListBox(_fullParametersWidget, paramLabelText, items, param._value.toString());
        }
            break;
        case CHECK_BOX: {
            widget = new EnrichedCheckBox(_fullParametersWidget, paramLabelText, getBoolValue(param._value));
        }
            break;
        case TABLE: {
            int nbCols = param._parameterSize.width();
            int nbRows = param._parameterSize.height();
            QStringList values = getNumList(param);
            EnrichedDecimalValueWidget* decimalValueWidget = new EnrichedTableWidget(_fullParametersWidget, paramLabelText, nbCols, nbRows, values, param._formatTemplate);
            decimalValueWidget->setPrecision(param._precision);
            widget = decimalValueWidget;
        }
            break;
        case FILE_SELECTOR_RELATIVE:
        case FILE_SELECTOR_ABSOLUTE: {
            widget = new EnrichedFileChooser(_fullParametersWidget, paramLabelText, param._show, param._value.toString());
        }
            break;
        case DIR_SELECTOR_RELATIVE:
        case DIR_SELECTOR_ABSOLUTE: {
            widget = new EnrichedFileChooser(_fullParametersWidget, paramLabelText, param._show, param._value.toString());
        }
            break;
        case UNKNOWN_SHOW:
        default:
            break;
        }
        if (widget) {
            widget->setObjectName("_WID_singleParameterForm");
            widget->hide();

            connect(widget, SIGNAL(signal_valueChanged(bool)), _fullParametersWidget, SLOT(slot_valueModified(bool)));

            if (!_valuesWidgets.contains(structName)) {
                QMap<QString, EnrichedFormWidget*> *structWidgets = new QMap<QString, EnrichedFormWidget*>();
                _valuesWidgets.insert(structName, *structWidgets);
            }
            _valuesWidgets[structName].insert(param._name, widget);
            _valueWidgetsByParamName.insert(param._name, widget);

            if (!_groupsWidgets.contains(structName)) {
                QMap<QString, QWidget*> *structGroupsWidgets = new QMap<QString, QWidget*>();
                _groupsWidgets.insert(structName, *structGroupsWidgets);
            }
            _groupsWidgets[structName].insert(param._name, currentGroup);

            currentGroup->layout()->addWidget(widget);
        }

    }

    /* hide last group */
    if (currentGroup) {
        currentGroup->hide();
    }

}

void MatisseParametersManager::translateHeaderButtons()
{
    ParametersHeaderButton *userHeaderButton = _headerButtonsByLevel.value(USER);
    userHeaderButton->setText(tr("Parametres utilisateur"));

    ParametersHeaderButton *advancedHeaderButton = _headerButtonsByLevel.value(ADVANCED);
    advancedHeaderButton->setText(tr("Parametres avances"));

    ParametersHeaderButton *expertHeaderButton = _headerButtonsByLevel.value(EXPERT);
    expertHeaderButton->setText(tr("Parametres mode expert"));
}

void MatisseParametersManager::slot_translateParameters()
{
    translateHeaderButtons();

    QList<QString> paramNames = _valueWidgetsByParamName.keys();
    foreach (QString paramName, paramNames) {
        EnrichedFormWidget *valueWidget = _valueWidgetsByParamName.value(paramName);
        QString translatedLabel = _dictionnaryLabels.getParamLabel(paramName);
        valueWidget->setLabelText(translatedLabel);
    }

    retranslateLevelGroups(USER);
    retranslateLevelGroups(ADVANCED);
    retranslateLevelGroups(EXPERT);
}

void MatisseParametersManager::slot_foldUnfoldLevelParameters()
{
    ParametersHeaderButton* headerButton = qobject_cast<ParametersHeaderButton*>(sender());
    //headerButton->slot_flip();
    ParameterLevel level = headerButton->getLevel();

    QWidget* paramContainer = _paramContainersByLevel.value(level);

    if (headerButton->getIsUnfolded()) {
        paramContainer->show();
    } else {
        paramContainer->hide();
    }
}

void MatisseParametersManager::retranslateLevelGroups(ParameterLevel level)
{
    QMap<QString, QGroupBox*> *levelGroups = _groupWidgetsByLevel.value(level);

    QList<QString> groupNames = levelGroups->keys();
    foreach (QString groupName, groupNames) {
        QGroupBox *groupWidget = levelGroups->value(groupName);
        QString translatedLabel = _dictionnaryLabels.getGroupLabel(groupName);
        groupWidget->setTitle(translatedLabel);
    }

}
void MatisseParametersManager::applyApplicationContext(bool isExpert, bool isProgramming)
{
    ParametersHeaderButton *advancedParamsHeader = _headerButtonsByLevel.value(ADVANCED);
    QWidget *advancedParamsContainer = _paramContainersByLevel.value(ADVANCED);

    ParametersHeaderButton *expertParamsHeader = _headerButtonsByLevel.value(EXPERT);
    QWidget *expertParamsContainer = _paramContainersByLevel.value(EXPERT);

    if (isExpert) {
        advancedParamsHeader->setIsUnfolded(isProgramming);
        expertParamsHeader->setIsUnfolded(isProgramming);
        expertParamsHeader->show();

        if (isProgramming) {
            advancedParamsContainer->show();
            expertParamsContainer->show();
        } else {
            advancedParamsContainer->hide();
            expertParamsContainer->hide();
        }

    } else {
        advancedParamsHeader->setIsUnfolded(false);
        advancedParamsContainer->hide();
        expertParamsHeader->hide();
        expertParamsContainer->hide();
    }
}

void MatisseParametersManager::toggleReadOnlyMode(bool isReadOnly)
{
    if (isReadOnly == _isReadOnlyMode) {
        return;
    }

    qDebug() << "Toggling parameter widgets to mode " << ((isReadOnly) ? "Read-Only" : "Editing");

    foreach (QString structName, _valuesWidgets.keys()) {
        QMap<QString, EnrichedFormWidget*> structParams = _valuesWidgets.value(structName);
        foreach (EnrichedFormWidget* widget, structParams.values()) {
            widget->setEnabled(!isReadOnly);
        }
    }

    _isReadOnlyMode = isReadOnly;
}

void MatisseParametersManager::pullDatasetParameters(KeyValueList &kvl)
{
    foreach (QString datasetParamName, _datasetParamNames) {
        if (kvl.getKeys().contains(datasetParamName)) {

            /* Get assembly template parameter value */
            QString value = getValue(DATASET_STRUCTURE, datasetParamName);

            /* Default value is not overriden if the parameter is not defined for the assembly */
            if (value != "") {
                kvl.set(datasetParamName, value);
            }
        }
    }
}

void MatisseParametersManager::pushPreferredDatasetParameters(KeyValueList kvl)
{
    _preferredDatasetParameters.insert(DATASET_PARAM_OUTPUT_DIR, kvl.getValue(DATASET_PARAM_OUTPUT_DIR));
    _preferredDatasetParameters.insert(DATASET_PARAM_OUTPUT_FILENAME, kvl.getValue(DATASET_PARAM_OUTPUT_FILENAME));

    foreach (QString prefParamName, _preferredDatasetParameters.keys()) {
        QString prefParamValue = _preferredDatasetParameters.value(prefParamName);

        /* override default value only if the preference value was defined */
        if (prefParamValue == "") {
            continue;
        }

        QWidget* paramWidget = _valuesWidgets.value(DATASET_STRUCTURE).value(prefParamName);

        if (!paramWidget) {
            qCritical() << QString("Widget for parameter '%1' is null").arg(prefParamName);
            continue;
        }

        EnrichedFormWidget* actualParamWidget = static_cast<EnrichedFormWidget *>(paramWidget);
        actualParamWidget->overrideDefaultValue(prefParamValue);
    }
}

void MatisseParametersManager::pushDatasetParameters(KeyValueList kvl)
{
    QString resultPath = kvl.getValue(DATASET_PARAM_OUTPUT_DIR);
    QString outputFile = kvl.getValue(DATASET_PARAM_OUTPUT_FILENAME);
    QString dataPath;
    QString navigationFile;

    bool isRealTime = false;
    if (kvl.getKeys().contains(DATASET_PARAM_DATASET_DIR)) {
        dataPath = kvl.getValue(DATASET_PARAM_DATASET_DIR);
        navigationFile = kvl.getValue(DATASET_PARAM_NAVIGATION_FILE);
    } else {
        isRealTime = true;
    }

    _jobExtraParameters.clear();

    /* mettre a jour valeur de parametre pour le dossier de sortie */
    EnrichedFormWidget* paramWidget;
    paramWidget = _valuesWidgets.value(DATASET_STRUCTURE).value(DATASET_PARAM_OUTPUT_DIR);
    paramWidget->setValue(resultPath);

    if (!_expectedParameters.contains(DATASET_PARAM_OUTPUT_DIR)) {
        _jobExtraParameters.insert(DATASET_PARAM_OUTPUT_DIR);
    }

    /* mettre a jour valeur de parametre pour le nom du fichier de sortie */
    paramWidget = _valuesWidgets.value(DATASET_STRUCTURE).value(DATASET_PARAM_OUTPUT_FILENAME);
    paramWidget->setValue(outputFile);

    if (!_expectedParameters.contains(DATASET_PARAM_OUTPUT_FILENAME)) {
        _jobExtraParameters.insert(DATASET_PARAM_OUTPUT_FILENAME);
    }

    if (!isRealTime) {
        /* mettre a jour valeur de parametre pour le chemin du dataset */
        paramWidget = _valuesWidgets.value(DATASET_STRUCTURE).value(DATASET_PARAM_DATASET_DIR);
        paramWidget->setValue(dataPath);


        if (!_expectedParameters.contains(DATASET_PARAM_DATASET_DIR)) {
            _jobExtraParameters.insert(DATASET_PARAM_DATASET_DIR);
        }

        /* mettre a jour valeur de parametre pour le fichier de navigation */
        paramWidget = _valuesWidgets.value(DATASET_STRUCTURE).value(DATASET_PARAM_NAVIGATION_FILE);
        paramWidget->setValue(navigationFile);

        if (!_expectedParameters.contains(DATASET_PARAM_NAVIGATION_FILE)) {
            _jobExtraParameters.insert(DATASET_PARAM_NAVIGATION_FILE);
        }
    }
}




bool MatisseParametersManager::saveParametersValues(QString entityName, bool isAssemblyTemplate)
{
    QString path;

    if (isAssemblyTemplate) {
        path = "xml/assemblies/parameters/";
        _jobExtraParameters.clear();
    } else {
        path = "xml/jobs/parameters/";
    }

    QString filename = path.append(entityName).append(".xml");
    bool saveStatus = writeParametersFile(filename, true);
    return saveStatus;
}

bool MatisseParametersManager::loadParameters(QString entityName, bool isAssemblyTemplate)
{
    QString path;

    if (isAssemblyTemplate) {
        path = "xml/assemblies/parameters/";
        _selectedAssembly = entityName;
    } else {
        path = "xml/jobs/parameters/";
    }

    QString filename = path.append(entityName).append(".xml");
    bool readStatus = readParametersFile(filename, isAssemblyTemplate);
    _fullParametersWidget->clearModifications();
    return readStatus;
}

void MatisseParametersManager::restoreParametersDefaultValues()
{
    qDebug() << "Restoring parameters values to default";

    foreach (QString structName, _valuesWidgets.keys()) {
        QMap<QString, EnrichedFormWidget*> structParams = _valuesWidgets.value(structName);
        foreach (EnrichedFormWidget* widget, structParams.values()) {
            widget->restoreDefaultValue();
        }
    }
}

void MatisseParametersManager::createJobParametersFile(QString assemblyName, QString jobName, KeyValueList kvl)
{
    qDebug() << QString("Creating job parameters file for job '%1'").arg(jobName);

    if (assemblyName != _selectedAssembly) {
        qCritical() << QString("Job owner assembly '%1 does not match currently selected assembly '%2'")
                       .arg(assemblyName).arg(_selectedAssembly);
        return;
    }

    pushDatasetParameters(kvl);

    QString jobParametersFilename = QString("xml/jobs/parameters/").append(jobName).append(".xml");
    writeParametersFile(jobParametersFilename);
}

bool MatisseParametersManager::writeParametersFile(QString parametersFilename, bool overwrite)
{

    /* creation de l'arborescence si nécessaire */
    QDir parentFolder =  QFileInfo(parametersFilename).dir();
    if (!parentFolder.exists()) {
        parentFolder.mkpath(".");
    }

    QFile parametersFile(parametersFilename);

    if (!overwrite && parametersFile.exists()) {
        qCritical() << QString("Job parameters file '%1' already exists").arg(parametersFilename);
        return false;
    }

    if (!parametersFile.open(QIODevice::WriteOnly)) {
        qCritical() << QString("Could not open job parameters file '%1' for writing").arg(parametersFilename);
        return false;
    }

    QXmlStreamWriter writer(&parametersFile);
    writer.setCodec("UTF-8");
    writer.setAutoFormatting(true);
    writer.writeStartDocument();
    writer.writeStartElement("JobParameters");
    writer.writeNamespace("http://www.w3.org/2001/XMLSchema-instance", "xsi");
    writer.writeAttribute("xsi:noNamespaceSchemaLocation", "../../../schemas/JobParameters.xsd");

    foreach(QString structName, _structures.keys()) {
        writer.writeStartElement("Structure");
        writer.writeAttribute("name", structName);
        Structure structure = _structures[structName];
        for (int noGroup = 0; noGroup < structure._groupsNames.length(); noGroup++) {
            QString groupName = structure._groupsNames.at(noGroup);
            if ( groupName != "") {
                writer.writeComment(groupName);
            }

            foreach(Parameter parameter, structure._parametersGroups.at(noGroup)._parameters) {
                QString parameterName = parameter._name;

                if (!_expectedParameters.contains(parameterName) && (!_jobExtraParameters.contains(parameterName))) {                    
                    qDebug() << QString("Parameter '%1' not expected, skipping...").arg(parameterName);
                    continue;
                }

                QString value = getValue(structName, parameterName);
                writer.writeStartElement("Parameter");
                writer.writeAttribute("name", parameterName);
                writer.writeCharacters(value);
                writer.writeEndElement();
            }
        }

        writer.writeEndElement();
    }

    writer.writeEndDocument();

    parametersFile.flush();
    parametersFile.close();

    return true;
}



bool MatisseParametersManager::readParametersFile(QString filename, bool isAssemblyTemplate)
{
    QString fileType = (isAssemblyTemplate) ? "assembly template" : "job";
    qDebug() << QString("Reading %1 parameters file %2").arg(fileType).arg(filename);

    QFile parametersFile(filename);
    if (!parametersFile.exists()) {
        qCritical() << QString("Parameters file '%1' not found").arg(filename);
        return false;
    }

    if (!parametersFile.open(QIODevice::ReadOnly)) {
        qCritical() << QString("Error opening parameters file '%1'").arg(filename);
        return false;
    }

    if (!isAssemblyTemplate) {
        _jobExtraParameters.clear();
    }

    QXmlStreamReader reader(&parametersFile);

    QString currentStructure;
    bool parsingOk = true;

    while(!reader.atEnd()) {        
        /* Read next element.*/
        QXmlStreamReader::TokenType token = reader.readNext();
        /* If token is just StartDocument, we'll go to next.*/
        if(token == QXmlStreamReader::StartDocument) {
           continue;
        }
        /* If token is StartElement, we'll see if we can read it.*/
        if(token == QXmlStreamReader::StartElement) {
            QString tagName = reader.name().toString();

            if (tagName == "Structure") {
                QXmlStreamAttributes attributes = reader.attributes();
                currentStructure = attributes.value("name").toString();

                if (!_structures.contains(currentStructure)) {
                    qCritical() << QString("Structure '%1' from parameters file not found in parameters dictionnary").arg(currentStructure);
                    currentStructure = "";
                    continue;
                }

            } else if (tagName == "Parameter") {
                QXmlStreamAttributes attributes = reader.attributes();
                QString paramName = attributes.value("name").toString();

                if (currentStructure.isEmpty()) {
                    qWarning() << QString("Parameter '%1' associated to unknown structure in parameters file, skipping...").arg(paramName);
                    continue;
                }

                if (!_structureByParameter.contains(paramName)) {
                    qCritical() << QString("Parameter '%1' from parameters file not found in dictionnary").arg(paramName);
                    continue;
                }

                QString dicoStructureName = _structureByParameter.value(paramName);

                if (currentStructure != dicoStructureName) {
                    qWarning() << QString("Parameter '%1' associated to '%2' structure in parameters file instead of '%3' in dictionnary")
                                  .arg(paramName).arg(currentStructure).arg(dicoStructureName);
                }

                bool isExtraDatasetParam = false;                

                if (!_expectedParameters.contains(paramName)) {
                    if (!isAssemblyTemplate && _datasetParamNames.contains(paramName)) {
                        qDebug() << QString("Extra parameter '%1' found in job parameters file. Keeping as dataset parameter").arg(paramName);
                        isExtraDatasetParam = true;
                    } else {
                        // Signaler incoherence et ignorer parametre
                        qWarning() << QString("Parameter '%1' found in parameters file is not referenced as expected by the assembly, skipping...")
                                      .arg(paramName);
                        continue;
                    }
                }

                QWidget* paramWidget = _valuesWidgets.value(dicoStructureName).value(paramName);

                if (!paramWidget) {
                    qCritical() << QString("Widget for parameter '%1' is null").arg(paramName);
                    continue;
                }

                QString value = reader.readElementText();

                EnrichedFormWidget* actualParamWidget = static_cast<EnrichedFormWidget *>(paramWidget);
                actualParamWidget->setValue(value);
                if (isExtraDatasetParam) {
                    _jobExtraParameters.insert(paramName);
                    actualParamWidget->show(); // montrer le parametre non defini dans l'assemblage
                }
            }

        }

        if (reader.hasError()) {
            qCritical() << "Error parsing parameters file:\n" << reader.errorString();
            parsingOk = false;
            break;
        }
    }

    reader.clear();
    parametersFile.close();

    return parsingOk;
}


bool MatisseParametersManager::getRange(Parameter param, QVariant &minValue, QVariant &maxValue)
{
    QString range = param._range;
    if (range.startsWith("{$")) {
        // recherche enums defini a l'exterieur
        // retourne premiere et derniere valeur
    } else if (_setRangeExpr.exactMatch(range)) {
        // recherche des enums definis a l'interieur // String obligatoirement ou int implicite
        // retourne premiere et derniere valeur
    } else if (_intervalRangeExpr.exactMatch(range)) {
        // recherche min max
        minValue = range.mid(1, range.size()-2).split(",").at(0);
        maxValue = range.mid(1, range.size()-2).split(",").at(1);
        if (range.startsWith("]")) {
            if (param._type == PAR_INT) {
                if (minValue.toString() != "-inf") {
                    minValue = minValue.toInt() + 1;
                }
            } else if ((param._type == PAR_FLOAT) || (param._type == PAR_DOUBLE)) {
                if (minValue.toString() != "-inf") {
                    minValue = minValue.toDouble() + _epsilon;
                }
            }
        }
        if (range.endsWith("[")) {
            if (param._type == PAR_INT) {
                if (maxValue.toString() != "inf") {
                    maxValue = maxValue.toInt() - 1;
                }
            } else if ((param._type == PAR_FLOAT) || (param._type == PAR_DOUBLE)) {
                if (!maxValue.toString().endsWith("inf")) {
                    maxValue = maxValue.toDouble() - _epsilon;
                }
            }

        }
    }

    return true;
}

QString MatisseParametersManager::getValue(QString structName, QString parameterName)
{
    QWidget * widget = _valuesWidgets.value(structName).value(parameterName);
    QString value;

    if (widget == NULL) {
        qDebug() << "Pas de widget pour" << structName << "->" << parameterName;
        // on est en user, on prend la valeur par defaut
        Structure currentStruct =  _structures.value(structName);
        foreach(ParametersGroup parametersGroup, currentStruct._parametersGroups) {
            qint32 indexParam = parametersGroup._parametersNames.indexOf(parameterName);
            if (indexParam > -1) {
                return parametersGroup._parameters[indexParam]._value.toString();
            }
        }
        return value;
    }
    else {
       value = (qobject_cast<EnrichedFormWidget *>(widget))->currentValue();
    }


    return value;
}


qint32 MatisseParametersManager::getIntValue(QVariant value)
{
    qint32 retValue;

    if (value.toString() == "-inf") {
        retValue = -InfInt;
    } else if (value.toString() == "inf") {
        retValue = InfInt;
    } else {
        retValue = value.toInt();
    }

    return retValue;
}

bool MatisseParametersManager::getBoolValue(QVariant value)
{
    bool retValue;

    if (value.toString() == "true") {
        retValue = true;
    } else if (value.toString() == "false") {
        retValue = false;
    } else {
        retValue = value.toBool();
    }

    return retValue;
}

qreal MatisseParametersManager::getDoubleValue(QVariant value)
{
    qreal retValue;

    if (value.toString() == "-inf") {
        retValue = -InfDouble;
    } else if (value.toString() == "inf") {
        retValue = InfDouble;
    } else {
        retValue = value.toDouble();
    }

    return retValue;
}

QStringList MatisseParametersManager::getNumList(Parameter param)
{
    QString valuesStr = param._value.toString();

    valuesStr = valuesStr.simplified().replace(" ", "");
//    valuesStr = valuesStr.mid(1, valuesStr.length()-2);

    valuesStr.replace("inf",_infStr);

    quint16 nbVars = param._parameterSize.width() * param._parameterSize.height();

    QStringList retValue;

    if (_matrixValuesExpr.exactMatch(valuesStr)) {
        retValue = valuesStr.split(";");
    }

    for (int noVar = retValue.length(); noVar < nbVars; noVar++) {
            retValue.append("0");
    }

    return retValue;
}

QStringList MatisseParametersManager::getEnums(Parameter param)
{
    QStringList ret;
    QString range = param._range;
    if (range.startsWith("{$")) {
        range = range.mid(2, range.size()-3).toUpper();
        if (_enums.contains(range)) {
            foreach(EnumValue value, _enums[range]._values) {
                ret.append(value._text);
            }
        }
    } else {
        range  = range.mid(1, range.size()-2);
        ret << range.split(",");
    }

    return ret;
}



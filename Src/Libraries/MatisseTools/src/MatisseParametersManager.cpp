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
    _selectedAssembly("")
{
    loadStaticCollections();
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

    _datasetParamNames.insert("output_dir");
    _datasetParamNames.insert("output_filename");
    _datasetParamNames.insert("dataset_dir");
    _datasetParamNames.insert("navFile");
}

void MatisseParametersManager::checkDictionnaryComplete()
{
    if (!_structures.contains("dataset_param")) {
        qFatal("Dictionnary does not contain mandatory 'dataset_param' structure");
    }

    Structure datasetStruct = _structures.value("dataset_param");
    QList<ParametersGroup> datasetGroups = datasetStruct._parametersGroups;

    bool outputDirParamFound = false;
    bool outputFilenameParamFound = false;
    bool datasetDirParamFound = false;
    bool navFileParamFound = false;

    bool allFound = false;

    // TODO changer algo pour iterer sur la collection _datasetParamNames
    foreach (ParametersGroup group, datasetGroups) {
        QStringList params = group._parametersNames;

        if (!outputDirParamFound) {
            if (params.contains("output_dir")) {
                outputDirParamFound = true;
            }
        }

        if (!outputFilenameParamFound) {
            if (params.contains("output_filename")) {
                outputFilenameParamFound = true;
            }
        }

        if (!datasetDirParamFound) {
            if (params.contains("dataset_dir")) {
                datasetDirParamFound = true;
            }
        }

        if (!navFileParamFound) {
            if (params.contains("navFile")) {
                navFileParamFound = true;
            }
        }

        allFound = outputDirParamFound && outputFilenameParamFound && datasetDirParamFound && navFileParamFound;
        if (allFound) {
            break;
        }
    }

    if (!allFound) {
        qFatal("One of the dataset params 'output_dir', 'output_filename', 'dataset_dir' or 'navFile' was not found in the dictionnary");
    }
}

bool MatisseParametersManager::readDictionnaryFile(QString xmlFilename)
{
    //if (!append) {
        _structures.clear();
//        _structuresNames.clear();
//        _enums.clear();
//        _fileInfo.clear();
    //}

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
        if (reader.readNextStartElement()) {
            QString name = reader.name().toString();
            // qDebug() << "START ELEMENT:" << name;
            attributes = reader.attributes();

            if (name == "Structures") {
                _dicoPublicationTimestamp = QDateTime::fromString(attributes.value("publicationTimestamp").toString(), "yyyy-MM-dd'T'hh:mm:ss");
            } else if (name == "Structure") {
                structureName = attributes.value("name" ).toString();
                // qDebug() << "Trouvé structure" << structureName;
                Structure newStruct;
                newStruct._name = structureName;
                newStruct._hasUserValues = false;
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
//            } else if (name == "Field") {
//            _fileInfo << attributes.value("name").toString();
//        }
        }

        if (reader.hasError()) {
            qWarning() << "An error occurred while reading the dictionnary file:\n" << reader.errorString();
        }
    }
    // qDebug() << "Fermeture fichier...";
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

    // Pas la peine de vérifier la présence de la clé (déjà vérifiée pour structure)
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
            qWarning() << QString("Param '%1' is already referenced as member of expected group '%2'").arg(paramName).arg(groupName);
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
        noGroup = _structures[structName]._groupsNames.size()-1;
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

    _structures[structName]._parametersGroups[noGroup]._parametersNames << parameter._name;
    _structures[structName]._parametersGroups[noGroup]._parameters.append(parameter);

    _parameters.insert(parameter._name, parameter);

    _structureByParameter.insert(parameter._name, structName);

    _groupByParameter.insert(parameter._name, groupName);

    /* Appending parameter name to the usage level list */
    QList<QString>* paramsForLevel;

    if (!_parametersByLevel.contains(level)) {
        paramsForLevel = new QList<QString>;
        _parametersByLevel.insert(level, paramsForLevel);
    } else {
        paramsForLevel = _parametersByLevel.value(level);
    }

    paramsForLevel->append(parameter._name);

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

//ParametersWidgetSkeleton *MatisseParametersManager::generateParametersWidget(QWidget *owner)
//{
//    qDebug() << "Building parameters widget";
//    _fullParametersWidget = new ParametersWidgetSkeleton(owner);
//    //eraseDialog();

//    // squeleton is created for all parameters in dictionnary
//    foreach (QString structureName, _structuresNames) {
//        ParametersWidgetSkeleton * dialogPart = createDialog(_fullParametersWidget, structureName, false);
//        if (dialogPart)
//        {
//            _fullParametersWidget->addWidget(dialogPart);
//            // pour transmettre le signal...
//            connect(dialogPart, SIGNAL(signal_valuesModified(bool)), _fullParametersWidget, SLOT(slot_valueModified(bool)));
//        }
//    }

//    return _fullParametersWidget;
//}

ParametersWidgetSkeleton *MatisseParametersManager::generateParametersWidget(QWidget *owner)
{
    qDebug() << "Building parameters widget";
    _fullParametersWidget = new ParametersWidgetSkeleton(owner);
    _fullParametersWidget->setObjectName("_WID_parametersWidget");

    generateLevelParametersWidget(USER, tr("Parametres utilisateur"));
    generateLevelParametersWidget(ADVANCED, tr("Parametres avances"));
    generateLevelParametersWidget(EXPERT, tr("Parametres mode expert"));

    return _fullParametersWidget;
}

void MatisseParametersManager::generateLevelParametersWidget(ParameterLevel level, QString levelHeader)
{
    QPushButton *levelHeaderButton = new QPushButton(_fullParametersWidget);
    levelHeaderButton->setObjectName("_PB_levelHeaderButton");
    levelHeaderButton->setText(levelHeader);
    levelHeaderButton->setIcon(QIcon(":/qss_icons/rc/branch_closed-on.png"));
    _fullParametersWidget->addWidget(levelHeaderButton);

    QWidget *levelContainer = new QWidget(_fullParametersWidget);
    levelContainer->setObjectName("_WID_levelParametersContainer");
    QVBoxLayout *levelContainerLayout = new QVBoxLayout();
    levelContainer->setLayout(levelContainerLayout);
    _fullParametersWidget->addWidget(levelContainer);

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

            currentGroup = new QGroupBox(group._text, _fullParametersWidget);
            QVBoxLayout* currentGroupLayout = new QVBoxLayout();
            currentGroupLayout->setContentsMargins(0, PARAM_GROUP_MARGIN_TOP, 0, PARAM_GROUP_MARGIN_BOTTOM);

            currentGroup->setLayout(currentGroupLayout);
            levelContainerLayout->addWidget(currentGroup);

            levelGroups.insert(groupName);
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
        QString paramLabel = tr(param._text.toLatin1());

        switch(param._show) {
        case LINE_EDIT: {
            widget = new EnrichedLineEdit(_fullParametersWidget, paramLabel, param._value.toString());
         }
            break;
        case SPIN_BOX: {
            QVariant minValue;
            QVariant maxValue;
            getRange(param, minValue, maxValue);
            widget = new EnrichedSpinBox(_fullParametersWidget, param._text, minValue.toString(), maxValue.toString(), param._value.toString());
        }
            break;
        case DOUBLE_SPIN_BOX: {
            QVariant minValue;
            QVariant maxValue;
            getRange(param, minValue, maxValue);
            EnrichedDecimalValueWidget* decimalValueWidget = new EnrichedDoubleSpinBox(_fullParametersWidget, param._text, minValue.toString(), maxValue.toString(), param._value.toString());
            decimalValueWidget->setPrecision(param._precision);
            widget = decimalValueWidget;
        }
            break;
        case COMBO_BOX: {
            QStringList items = getEnums(param);
            widget = new EnrichedComboBox(_fullParametersWidget, paramLabel, items, param._value.toString());
        }
            break;
        case LIST_BOX: {
            QStringList items = getEnums(param);
            widget = new EnrichedListBox(_fullParametersWidget, param._text, items, param._value.toString());
        }
            break;
        case CHECK_BOX: {
            widget = new EnrichedCheckBox(_fullParametersWidget, param._text, getBoolValue(param._value));
        }
            break;
        case TABLE: {
            int nbCols = param._parameterSize.width();
            int nbRows = param._parameterSize.height();
            QStringList values = getNumList(param);
            EnrichedDecimalValueWidget* decimalValueWidget = new EnrichedTableWidget(_fullParametersWidget, param._text, nbCols, nbRows, values, param._formatTemplate);
            decimalValueWidget->setPrecision(param._precision);
            widget = decimalValueWidget;
        }
            break;
        case FILE_SELECTOR_RELATIVE:
        case FILE_SELECTOR_ABSOLUTE: {
            widget = new EnrichedFileChooser(_fullParametersWidget, param._text, param._show, param._value.toString());
        }
            break;
        case DIR_SELECTOR_RELATIVE:
        case DIR_SELECTOR_ABSOLUTE: {
            widget = new EnrichedFileChooser(_fullParametersWidget, param._text, param._show, param._value.toString());
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

void MatisseParametersManager::updateDatasetParametersValues(KeyValueList kvl)
{
    QString resultPath = kvl.getValue("resultPath");
    QString outputFile = kvl.getValue("outputFile");
    QString dataPath;
    QString navigationFile;

    bool isRealTime = false;
    if (kvl.getKeys().contains("dataPath")) {
        dataPath = kvl.getValue("dataPath");
        navigationFile = kvl.getValue("navigationFile");
    } else {
        isRealTime = true;
    }

    _jobExtraParameters.clear();

    /* mettre a jour valeur de parametre pour le dossier de sortie */
    EnrichedFormWidget* paramWidget;
    paramWidget = _valuesWidgets.value("dataset_param").value("output_dir");
    paramWidget->setValue(resultPath);

    if (!_expectedParameters.contains("output_dir")) {
        _jobExtraParameters.insert("output_dir");
    }

    /* mettre a jour valeur de parametre pour le nom du fichier de sortie */
    paramWidget = _valuesWidgets.value("dataset_param").value("output_filename");
    paramWidget->setValue(outputFile);

    if (!_expectedParameters.contains("output_filename")) {
        _jobExtraParameters.insert("output_filename");
    }

    if (!isRealTime) {
        /* mettre a jour valeur de parametre pour le chemin du dataset */
        paramWidget = _valuesWidgets.value("dataset_param").value("dataset_dir");
        paramWidget->setValue(dataPath);


        if (!_expectedParameters.contains("dataset_dir")) {
            _jobExtraParameters.insert("dataset_dir");
        }

        /* mettre a jour valeur de parametre pour le fichier de navigation */
        paramWidget = _valuesWidgets.value("dataset_param").value("navFile");
        paramWidget->setValue(navigationFile);

        if (!_expectedParameters.contains("navFile")) {
            _jobExtraParameters.insert("navFile");
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

    updateDatasetParametersValues(kvl);

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
        //out << "\t</Structure>\n\n";
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
        if (reader.readNextStartElement()) {
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

//ParametersWidgetSkeleton * MatisseParametersManager::createDialog(QWidget* owner, QString structName, bool user) {
    // all widgets in dialog are initially hidden

//    QString structUserExpert = structName /*+ QString("%1").arg(user)*/;
//    if (_dialogs.contains(structUserExpert)) {
//        return _dialogs[structUserExpert];
//    }

//    if (!_structures.contains(structName)) {
//        return NULL;
//    }

//    if ((!_structures[structName]._hasUserValues) && user) {
//        return NULL;
//    }

//    QList<QLabel*> textsForAlignment;
//    QList<QGroupBox *> groupsForAlignement;
//    // on cree l'interface...
//    ParametersWidgetSkeleton * newDialog = new ParametersWidgetSkeleton(owner);

//    quint32 maxWidth = 0;
//    QFontMetrics metrics(QLabel().font());
//    foreach(ParametersGroup parameterGroup, _structures[structName]._parametersGroups) {
//        if ((!parameterGroup._hasUserValues) && user) {
//            continue;
//        }
//        QString groupLabel = tr(parameterGroup._text.toLatin1());
//        QGroupBox * groupBox = new QGroupBox(groupLabel, newDialog);
//        groupBox->setSizePolicy(QSizePolicy(QSizePolicy::Minimum, QSizePolicy::Minimum));
//        groupsForAlignement << groupBox;
//        QVBoxLayout * groupBoxLayout = new QVBoxLayout();

//        foreach(Parameter param, parameterGroup._parameters) {
//            if ((!param._userModify) && user) {
//                continue;
//            }
//            QHBoxLayout * parameterLayout = new QHBoxLayout(newDialog);
//            EnrichedFormWidget * widget = NULL;
//            QString paramLabel = tr(param._text.toLatin1());

//            switch(param._show) {
//            case LINE_EDIT: {
//                widget = new EnrichedLineEdit(newDialog, paramLabel, param._value.toString());
//             }
//                break;
//            case SLIDER: {
//                // A faire en enriched....
////                QSlider * curWid = new QSlider(Qt::Horizontal);
////                QVariant minValue;
////                QVariant maxValue;
////                getRange(param, minValue, maxValue);
////                curWid->setRange(minValue.toInt(), maxValue.toInt());
////                curWid->setValue(getIntValue(param._value));
////                widget = curWid;
//            }
//                qCritical() << "Slider parameter show not handled, widget will be null";
//                break;
//            case SPIN_BOX: {
//                QVariant minValue;
//                QVariant maxValue;
//                getRange(param, minValue, maxValue);
//                widget = new EnrichedSpinBox(newDialog, param._text, minValue.toString(), maxValue.toString(), param._value.toString());
//            }
//                break;
//            case DOUBLE_SPIN_BOX: {
//                QVariant minValue;
//                QVariant maxValue;
//                getRange(param, minValue, maxValue);
//                widget = new EnrichedDoubleSpinBox(newDialog, param._text, minValue.toString(), maxValue.toString(), param._value.toString());
//            }
//                break;
//            case COMBO_BOX: {
//                QStringList items = getEnums(param);
//                widget = new EnrichedComboBox(newDialog, paramLabel, items, param._value.toString());
//            }
//                break;
//            case LIST_BOX: {
//                QStringList items = getEnums(param);
//                widget = new EnrichedListBox(newDialog, param._text, items, param._value.toString());
//            }
//                break;
//            case CHECK_BOX: {
//                widget = new EnrichedCheckBox(newDialog, param._text, getBoolValue(param._value));
//            }
//                break;
//            case TABLE: {
//                int nbCols = param._parameterSize.width();
//                int nbRows = param._parameterSize.height();
//                QStringList values = getNumList(param);
//                widget = new EnrichedTableWidget(newDialog, param._text, nbCols, nbRows, values);
//            }
//                break;
//            case FILE_SELECTOR_RELATIVE:
//            case FILE_SELECTOR_ABSOLUTE: {
//                widget = new EnrichedFileChooser(newDialog, param._text, param._show, param._value.toString());
//            }
//                break;
//            case DIR_SELECTOR_RELATIVE:
//            case DIR_SELECTOR_ABSOLUTE: {
//                widget = new EnrichedFileChooser(newDialog, param._text, param._show, param._value.toString());
//            }
//                break;
//            case UNKNOWN_SHOW:
//            default:
//                break;
//            }
//            if (widget) {
//                widget->hide();

//                connect(widget, SIGNAL(signal_valueChanged(bool)), newDialog, SLOT(slot_valueModified(bool)));

//                if (!_valuesWidgets.contains(structName)) {
//                    QMap<QString, EnrichedFormWidget*> *structWidgets = new QMap<QString, EnrichedFormWidget*>();
//                    _valuesWidgets.insert(structName, *structWidgets);
//                }
//                _valuesWidgets[structName].insert(param._name, widget);

//                if (!_groupsWidgets.contains(structName)) {
//                    QMap<QString, QWidget*> *structGroupsWidgets = new QMap<QString, QWidget*>();
//                    _groupsWidgets.insert(structName, *structGroupsWidgets);
//                }
//                _groupsWidgets[structName].insert(param._name, groupBox);
//                QLabel * widLabel = new QLabel(param._text + ":");
//                quint32 currentWidth = metrics.width(param._text + ":");

//                maxWidth = qMax(maxWidth, currentWidth);
//                //parameterLayout->addWidget(widLabel);

////                parameterLayout->addWidget(widget);
////                parameterLayout->addStretch();
//                groupBoxLayout->addWidget(widget);

//                qDebug() << "Add parameter layout";

//                //groupBoxLayout->addLayout(parameterLayout);

//                textsForAlignment << widLabel;
//            } else {
//                parameterLayout->deleteLater();
//            }
//        }

//        qDebug() << "Set group box layout";
//        groupBox->setLayout(groupBoxLayout);
//        groupBox->hide();

//        newDialog->addWidget(groupBox);

//    }
//    // realignement textes...
//    // recalcul...
//    foreach(QLabel * label, textsForAlignment) {
//        label->setMinimumWidth(maxWidth);
//    }

//    _dialogs[structUserExpert] = newDialog;

//    return newDialog;
//    return NULL;
//}


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



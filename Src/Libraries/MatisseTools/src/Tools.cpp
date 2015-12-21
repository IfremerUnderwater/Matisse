#include "Tools.h"

using namespace MatisseCommon;
using namespace MatisseTools;

QMap<QString, ParameterType> Tools::_enumTypes;
QMap<QString, ParameterShow> Tools::_enumShows;
QRegExp Tools::_intervalRangeExpr("^(\\[|\\])((\\-?\\d+((\\.)\\d+)?)|(-inf)),((\\-?\\d+((\\.)\\d+)?)|(\\+?inf))(\\[|\\])$");
QRegExp Tools::_setRangeExpr("^\\{\\w+(,\\w+)*\\}$");

QRegExp Tools::_matrixExpr("^\\(\\d+(,\\d+)?\\)$");
QRegExp Tools::_matrixValuesExpr("^\\-?\\d+((\\.)\\d+)?(;\\-?\\d+((\\.)\\d+)?)*$");

double Tools::_epsilon = 0.000001;
QString Tools::_infStr = QString::number(quint64(InfInt));

Tools::Tools(/*QWidget *parent*/)/* :
    QDialog(parent),
    ui(new Ui::Wrapper)*/
{
//    ui->setupUi(this);
//    _currentDialog = 0;

    fillStaticValues();
    //    readParametersFile("datas/Datas.xml");
    //QList<QVariant> args;
    //args << 1 << "toto" << 2;
//    qDebug() << "Nombre de structures=" << _structures.size();
//    qDebug() << "Nombre d'enums=" << _enums.size();
//    if (_structures.size()>0) {
//        createDialog(_structures.keys().at(0));
//    }
}

Tools::~Tools()
{
    qDebug() << "Delete wrapper IHM";
    foreach(QWidget * dialog, _dialogs.values()) {
        if (dialog) {
            dialog->deleteLater();
        }
    }
    _dialogs.clear();

//    delete ui;
}

bool Tools::readParametersModelFile(QString xmlFilename, bool append)
{
    if (!append) {
        _structures.clear();
        _structuresNames.clear();
        _enums.clear();
        _fileInfo.clear();
    }

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
    QString groupName = "";
    QString enumsName;
    while (!reader.atEnd()) {
        if (reader.readNextStartElement()) {
            QString name = reader.name().toString();
            // qDebug() << "START ELEMENT:" << name;
            attributes = reader.attributes();

            if (name == "MatisseInit") {
                _version = attributes.value("version").toString();
            } else if (name == "Structure") {
                structureName = attributes.value("name" ).toString();
                // qDebug() << "Trouvé structure" << structureName;
                Structure newStruct;
                newStruct._name = structureName;
                newStruct._hasUserValues = false;
                _structures.insert(structureName, newStruct);
                _structuresNames << structureName;
            } else if (name == "ParametersGroup") {
                groupName = attributes.value("name" ).toString();
            } else if (name == "Parameter") {
                // qDebug() << "Add Parameter:" << structureName << groupName << attributes.value("name");
                addParameter(structureName, groupName, attributes);
            } else if (name == "Enum") {
                enumsName = attributes.value("name" ).toString();
                // qDebug() << "Trouvé enum" << enumsName;
                Enums newEnums;
                newEnums._name = enumsName;
                _enums.insert(enumsName, newEnums);
            } else if (name == "EnumValue") {
                addEnum(enumsName, attributes);
            } else if (name == "Field") {
            _fileInfo << attributes.value("name").toString();
        }
        }
    }
    // qDebug() << "Fermeture fichier...";
    inputFile.close();

    return (_structures.size() > 0);
}

bool Tools::readUserParametersFile(QString xmlFilename, QString xmlModelFilename)
{
    if (!xmlModelFilename.isEmpty()) {
        if (!readParametersModelFile(xmlModelFilename)) {
            return false;
        }
    }

    MatisseParameters  mosaicParameters(xmlFilename);
    if (mosaicParameters.lastErrorStr() != "") {
        qDebug() << "Parameters error =" << mosaicParameters.lastErrorStr();
        return false;
    }

    foreach(QString structureName, _structures.keys()) {
        Structure structure = _structures.value(structureName);
        for(int noGroup = 0; noGroup < structure._parametersGroups.length(); noGroup++) {
            ParametersGroup group = structure._parametersGroups.at(noGroup);
            for(int noParam = 0; noParam < group._parameters.length(); noParam++) {
                QString parameterName = group._parametersNames.at(noParam);
                if (mosaicParameters.containsParam(structureName, parameterName)) {
                    QString value = mosaicParameters.getStringParamValue(structureName, parameterName);
                    _structures[structureName]._parametersGroups[noGroup]._parameters[noParam]._value = value;
                }
             }
        }
    }

    return true;
}



void Tools::slot_saveProcess()
{
}

void Tools::eraseDialog()
{
    foreach (QString groupKey, _valuesWidgets.keys()) {
        QHash<QString, QWidget*> group = _valuesWidgets.take(groupKey);
        foreach (QString key, group.keys()) {
            group.take(key)->deleteLater();
        }
    }
    _valuesWidgets.clear();

    _dialogs.clear();
}

ParametersWidgetSkeleton *Tools::createFullParametersDialog(bool user) {
    ParametersWidgetSkeleton * fullParametersWidget = new ParametersWidgetSkeleton();
    eraseDialog();
    foreach (QString structureName, _structuresNames) {
        ParametersWidgetSkeleton * dialogPart = createDialog(structureName, user);
        if (dialogPart)
        {
            fullParametersWidget->addWidget(dialogPart);
            // pour transmettre le signal...
            connect(dialogPart, SIGNAL(signal_valuesModified(bool)), fullParametersWidget, SLOT(slot_valueModified(bool)));
        }
    }

    return fullParametersWidget;
}

QString Tools::getModelVersion()
{
    return _version;
}

ParametersWidgetSkeleton * Tools::createDialog(QString structName, bool user) {
    QString structUserExpert = structName /*+ QString("%1").arg(user)*/;
    if (_dialogs.contains(structUserExpert)) {
        return _dialogs[structUserExpert];
    }

    if (!_structures.contains(structName)) {
        return NULL;
    }

    if ((!_structures[structName]._hasUserValues) && user) {
        return NULL;
    }

    QList<QLabel*> textsForAlignment;
    QList <QGroupBox *> groupsForAlignement;
    // on cree l'interface...
    ParametersWidgetSkeleton * newDialog = new ParametersWidgetSkeleton();

    quint32 maxWidth = 0;
    QFontMetrics metrics(QLabel().font());
    foreach(ParametersGroup parameterGroup, _structures[structName]._parametersGroups) {
        if ((!parameterGroup._hasUserValues) && user) {
            continue;
        }
        QString groupName = parameterGroup._name;
        QGroupBox * groupBox = new QGroupBox(groupName);
        groupBox->setSizePolicy(QSizePolicy(QSizePolicy::Minimum, QSizePolicy::Minimum));
        groupsForAlignement << groupBox;
        QVBoxLayout * groupBoxLayout = new QVBoxLayout();
        foreach(Parameter param, parameterGroup._parameters) {
            if ((!param._userModify) && user) {
                continue;
            }
            QHBoxLayout * parameterLayout = new QHBoxLayout();
            EnrichedFormWidget * widget = 0;
            switch(param._show) {
            case LINE_EDIT: {
                widget = new EnrichedLineEdit(0, param._text, param._value.toString());
             }
                break;
            case SLIDER: {
                // A faire en enriched....
//                QSlider * curWid = new QSlider(Qt::Horizontal);
//                QVariant minValue;
//                QVariant maxValue;
//                getRange(param, minValue, maxValue);
//                curWid->setRange(minValue.toInt(), maxValue.toInt());
//                curWid->setValue(getIntValue(param._value));
//                widget = curWid;
            }
                break;
            case SPIN_BOX: {
                QVariant minValue;
                QVariant maxValue;
                getRange(param, minValue, maxValue);
                widget = new EnrichedSpinBox(0, param._text, minValue.toString(), maxValue.toString(), param._value.toString());
            }
                break;
            case DOUBLE_SPIN_BOX: {
                QVariant minValue;
                QVariant maxValue;
                getRange(param, minValue, maxValue);
                widget = new EnrichedDoubleSpinBox(0, param._text, minValue.toString(), maxValue.toString(), param._value.toString());
            }
                break;
            case COMBO_BOX: {
                QStringList items = getEnums(param);
                widget = new EnrichedComboBox(0, param._text, items, param._value.toString());
            }
                break;
            case LIST_BOX: {
                QStringList items = getEnums(param);
                widget = new EnrichedListBox(0, param._text, items, param._value.toString());
            }
                break;
            case CHECK_BOX: {
                widget = new EnrichedCheckBox(0, param._text, getBoolValue(param._value));
            }
                break;
            case TABLE: {
                int nbCols = param._parameterSize.width();
                int nbRows = param._parameterSize.height();
                QStringList values = getNumList(param);
                widget = new EnrichedTableWidget(0, param._text, nbCols, nbRows, values);
            }
                break;
            case FILE_SELECTOR_RELATIVE:
            case FILE_SELECTOR_ABSOLUTE: {
                widget = new EnrichedFileChooser(0, param._text, tr("Fichier"), param._show, param._value.toString());
            }
                break;
            case DIR_SELECTOR_RELATIVE:
            case DIR_SELECTOR_ABSOLUTE: {
                widget = new EnrichedFileChooser(0, param._text, tr("Repertoire"), param._show, param._value.toString());
            }
                break;
            case UNKNOWN_SHOW:
            default:
                break;
            }
            if (widget) {
                connect(widget, SIGNAL(signal_valueChanged(bool)), newDialog, SLOT(slot_valueModified(bool)));
                _valuesWidgets[structName].insert(param._name, widget);
                QLabel * widLabel = new QLabel(param._text + ":");
                quint32 currentWidth = metrics.width(param._text + ":");

                maxWidth = qMax(maxWidth, currentWidth);
                //parameterLayout->addWidget(widLabel);
                parameterLayout->addWidget(widget);
                parameterLayout->addStretch();
                groupBoxLayout->addLayout(parameterLayout);
                textsForAlignment << widLabel;
            } else {
                parameterLayout->deleteLater();
            }
        }

        groupBox->setLayout(groupBoxLayout);

    newDialog->addWidget(groupBox);

    }
    // realignement textes...
    // recalcul...
    foreach(QLabel * label, textsForAlignment) {
        label->setMinimumWidth(maxWidth);
    }

    _dialogs[structUserExpert] = newDialog;

    return newDialog;
}

void Tools::generateParametersFile(QString filename, QString modelVersion, KeyValueList comments)
{
    QFile xmlOut(filename);
    QDir parentFolder =  QFileInfo(filename).dir();
    if (!parentFolder.exists()) {
        parentFolder.mkpath(".");
    }


    xmlOut.open(QIODevice::WriteOnly);
    QTextStream out(&xmlOut);

    out << "<?xml version=\"1.0\" encoding=\"UTF-8\"?>\n";
    out << "\n";

    out << "<mosaic_settings xmlns:xsi=\"http://www.w3.org/2001/XMLSchema-instance\" xsi:noNamespaceSchemaLocation=\"../models/parameters/Parameters_V" + modelVersion + ".xsd\">\n";
    out << "\n";

    // Ajout des descripteurs
    out << "<fileDescriptor>\n";
    foreach(QString key, comments.getKeys()) {
        QStringList parts = comments.getValue(key).split("\n");
        if (parts.size() > 1) {
            out << QString("<%1>\n").arg(key);
            for(int index = 0; index < parts.size(); index++) {
                out << parts[index] + "\n";
            }
            out << QString("</%1>\n").arg(key);
        } else {
            out << QString("<%1>%2</%1>\n").arg(key).arg(parts[0]);
        }
    }

    out << "</fileDescriptor>\n";
    out << "\n";

    foreach(QString structName, _structures.keys()) {
        out << "<" << structName << ">\n";
        Structure structure = _structures[structName];
        for (int noGroup = 0; noGroup < structure._groupsNames.length(); noGroup++) {
            QString groupName = structure._groupsNames.at(noGroup);
            if ( groupName != "") {
                out << "<!--" << groupName << "-->\n";
            }

            foreach(Parameter parameter, structure._parametersGroups.at(noGroup)._parameters) {
                QString parameterName = parameter._name;
                QString value = getValue(structName, parameterName);
                out << "<" << parameterName << ">" << value <<  "</" << parameterName << ">\n";

            }
            out << "\n";
        }

        out << "</" << structName << ">\n\n";
    }

    out << "</mosaic_settings>";
    out.flush();
    xmlOut.close();
}

QStringList Tools::structureNames()
{
    return _structures.keys();
}

bool Tools::addParameter(QString structName, QString groupName, QXmlStreamAttributes attributes)
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
        newGroup._hasUserValues = false;
        _structures[structName]._parametersGroups.append(newGroup);
        noGroup = _structures[structName]._groupsNames.size()-1;
    }

    Parameter parameter;
    QString dummy;
    QScriptEngine scriptEngine;
    QScriptValue scriptValue;

    parameter._name = attributes.value("name").toString();
    scriptValue = scriptEngine.evaluate(attributes.value("user").toString().toLower());
    parameter._userModify = false;
    if (scriptValue.isBool()) {
        parameter._userModify = scriptValue.toBool();
        if (parameter._userModify) {
            _structures[structName]._hasUserValues = true;
            _structures[structName]._parametersGroups[noGroup]._hasUserValues = true;
        }
    }
    parameter._text = attributes.value("text").toString();
    parameter._suffix = attributes.value("suffix").toString();
    dummy = attributes.value("type").toString().toLower();
    if (!_enumTypes.contains(dummy)) {
        dummy = "unknown";
    }
    parameter._type = _enumTypes.value(dummy);

    dummy = attributes.value("show").toString().toLower();
    if (!_enumShows.contains(dummy)) {
        dummy = "unknown";
    }

    parameter._show = _enumShows.value(dummy);

    // double spin...
    if (((parameter._type == PAR_DOUBLE) || (parameter._type == PAR_FLOAT)) && (parameter._show == SPIN_BOX)) {
        parameter._show = _enumShows.value("spinDouble");
    }

    parameter._parameterSize = QSize(1,1);
    dummy = attributes.value("size").toString().simplified().replace(" ","");
    if (_matrixExpr.exactMatch(dummy)) {
        dummy = dummy.mid(1, dummy.size()-2);
        parameter._parameterSize.setWidth(dummy.split(",").at(0).toInt());
        parameter._parameterSize.setHeight(dummy.split(",").at(1).toInt());
    }

    parameter._range = attributes.value("range").toString().simplified().replace(" ","");

    parameter._value = attributes.value("default").toString();

    scriptValue = scriptEngine.evaluate(attributes.value("mandatory").toString().toLower());
    parameter._required = true;
    if (scriptValue.isBool()) {
        parameter._required = scriptValue.toBool();
    }

    _structures[structName]._parametersGroups[noGroup]._parametersNames << parameter._name;
    _structures[structName]._parametersGroups[noGroup]._parameters.append(parameter);

    return true;

}

bool Tools::addEnum(QString enumsName, QXmlStreamAttributes attributes)
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

void Tools::fillStaticValues(bool refill)
{
    if ((_enumTypes.size() > 0) && (_enumShows.size() > 0) && (!refill)) {
        return;
    }
    _enumTypes.clear();
    _enumTypes.insert("unknown", UNKNOWN_TYPE);
    _enumTypes.insert("integer", PAR_INT);
    _enumTypes.insert("float", PAR_FLOAT);
    _enumTypes.insert("double", PAR_DOUBLE);
    _enumTypes.insert("string", PAR_STRING);
    _enumTypes.insert("bool", PAR_BOOL);

    _enumShows.clear();
    _enumShows.insert("unknown", UNKNOWN_SHOW);
    _enumShows.insert("line", LINE_EDIT);
    _enumShows.insert("combo", COMBO_BOX);
    _enumShows.insert("list", LIST_BOX);
    _enumShows.insert("spin", SPIN_BOX);
    _enumShows.insert("spinDouble", DOUBLE_SPIN_BOX);
    _enumShows.insert("check", CHECK_BOX);
    _enumShows.insert("table", TABLE);
    _enumShows.insert("slider", SLIDER);
    _enumShows.insert("file", FILE_SELECTOR_ABSOLUTE);
    _enumShows.insert("file:relative", FILE_SELECTOR_RELATIVE);
    _enumShows.insert("file:absolute", FILE_SELECTOR_ABSOLUTE);
    _enumShows.insert("dir", DIR_SELECTOR_ABSOLUTE);
    _enumShows.insert("dir:relative", DIR_SELECTOR_RELATIVE);
    _enumShows.insert("dir:absolute", DIR_SELECTOR_ABSOLUTE);

    return;

}

bool Tools::getRange(Parameter param, QVariant &minValue, QVariant &maxValue)
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

QStringList Tools::getEnums(Parameter param)
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

QString Tools::getValue(QString structName, QString parameterName)
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

void Tools::fillValuesFromDialog(QString structName)
{
}

qint32 Tools::getIntValue(QVariant value)
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

bool Tools::getBoolValue(QVariant value)
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

qreal Tools::getDoubleValue(QVariant value)
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

QStringList Tools::getNumList(Parameter param)
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


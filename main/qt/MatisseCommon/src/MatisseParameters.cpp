#include "MatisseParameters.h"

using namespace MatisseCommon;

QRegExp MatisseParameters::_boolRegExpTrue("^(true|1)$");
QRegExp MatisseParameters::_boolRegExpFalse("^(false|0)$");

MatisseParameters::MatisseParameters(QString xmlFilename)
    : pFileInfo(NULL)
{
    _lastErrorStr = "";
    if (xmlFilename.trimmed() != "") {
        loadFile(xmlFilename);
    }
}

MatisseParameters::~MatisseParameters()
{
    if (pFileInfo!=NULL)
        delete pFileInfo;
}

bool MatisseParameters::loadFile(QString xmlFilename)
{
    _hashValues.clear();

    xmlFilename = xmlFilename.simplified().trimmed();
    if (xmlFilename == "") {
        _lastErrorStr = "Xml file not defined";
        return false;
    }

    QFile inputFile(xmlFilename);
    pFileInfo = new QFileInfo(inputFile);
    if (!inputFile.exists()) {
        _lastErrorStr = "File " + xmlFilename + " not found.";
        return false;
    }

    if (!inputFile.open(QIODevice::ReadOnly)) {
        _lastErrorStr = "Cannot open file " + xmlFilename + ".";
        return false;
    }

    QXmlStreamReader reader(&inputFile);

    int level = -99; //-99: pas de fichier mosaic0: avant debut fichier, 1: avant debut struct, 2: avant debut var, 3: avant debut val
    //    bool start = true;
    QString structName;
    QString paramName;
    bool ok = true;
    while ((!reader.atEnd()) && ok) {
        QXmlStreamReader::TokenType type = reader.readNext();
        QString name = reader.name().toString().trimmed();

        switch(type) {
        case QXmlStreamReader::StartElement:
        {
            if (name == "mosaic_settings") {
                level = 1;
            } else {
                switch (level) {
                case 1: {
                    // on ajoute une structure...
                    structName = name;
                    _hashValues.insert(structName,QHash<QString, QString>());
                    level = 2;
                }
                    break;
                case 2: {
                    // on defini un parametre
                    paramName = name;
                    level = 3;
                }
                    break;
                default: {
                    // on ne doit pas se trouver là...
                    _lastErrorStr = "XML file not compliant: start element found where value element expected";
                    ok = false;
                }
                    break;
                }
            }
        }
            break;
        case QXmlStreamReader::Characters:
        {
            switch (level) {
            case 3: {
                // on ajoute une valeur de parametre...
                // ou on complete pour les variables sur plusieurs lignes (cas matrice...)
                // on ajoute une virgule entre les valeurs
                // si autre cas, on traitera le moment venu...
                QString value = reader.text().toString();
                QString oldValue = "";
                if (_hashValues[structName].contains(paramName)) {
                    oldValue = _hashValues[structName].value(paramName, "") + ", ";
                }
                _hashValues[structName].insert(paramName, oldValue + value);
            }
                break;
            default: {
                // on ne doit pas se trouver là...
                // caracteres parasites???
                //_lastErrorStr = "XML file not compliant: value without parameter";
                //ok = false;
            }
                break;
            }
        }
            break;
        case QXmlStreamReader::EndElement:
        {
            switch (level) {
            case 3: {
                if (name != paramName) {
                    _lastErrorStr = "XML file not compliant: parameter begin without end";
                    ok = false;
                } else {
                    level--;
                }
            }
                break;
            case 2: {
                if (name != structName) {
                    _lastErrorStr = "XML file not compliant: struct begin without end";
                    ok = false;
                } else {
                    level--;
                }
            }
                break;
            case 1: {
                if (name != "mosaic_settings") {
                    _lastErrorStr = "XML file not compliant: mosaic_settings begin without end";
                    ok = false;
                } else {
                    level--;
                }
            }
                break;

            default: {
                _lastErrorStr = "XML file not compliant: format error";
                ok = false;
            }
                break;
            }
        }
            break;

        default:
            break;
        }

    }

    inputFile.close();

    return ok;
}

QFileInfo *MatisseParameters::getXmlFileInfo()
{
    return pFileInfo;
}

QString MatisseParameters::lastErrorStr()
{
    return _lastErrorStr;
}

QString MatisseParameters::dumpStructures()
{
    QString dump;

    foreach(QString structName, _hashValues.keys()) {
        dump.append("Structure :" + structName + "\n");
        QHash<QString, QString> params = _hashValues.value(structName);
        foreach(QString paramName, params.keys()) {
            dump.append("\t\tParam:" + paramName + ": " + params.value(paramName) + "\n");
        }
        dump.append("\n");
    }

    return dump;
}

bool MatisseParameters::containsParam(QString paramGroupName, QString paramName)
{
    if (_hashValues.find(paramGroupName) != _hashValues.end()) {
        QHash<QString, QString> params = _hashValues.value(paramGroupName);
        if (params.find(paramName) != params.end()) {
            return true;
        }
    }
    return false;

}

qint64 MatisseParameters::getIntParamValue(QString paramGroupName, QString paramName, bool &ok)
{
    QString value = _hashValues.value(paramGroupName,QHash<QString,QString>()).value(paramName,"").trimmed();
    if (value.endsWith("inf")) {
        value.replace("inf", QString("%1").arg(InfInt));
    }

    return value.toLongLong(&ok);
}

bool MatisseParameters::getBoolParamValue(QString paramGroupName, QString paramName, bool &ok)
{
    QString value = _hashValues.value(paramGroupName,QHash<QString,QString>()).value(paramName,"").trimmed().toLower();

    bool retValue = false;

    if (_boolRegExpTrue.exactMatch(value)) {
        retValue = true;
        ok = true;
    } else if (_boolRegExpFalse.exactMatch(value)) {
        ok = true;
    } else {
        ok = false;
    }

    return retValue;
}

qreal MatisseParameters::getDoubleParamValue(QString paramGroupName, QString paramName, bool &ok)
{
    QString value = _hashValues.value(paramGroupName,QHash<QString,QString>()).value(paramName,"").trimmed();
    if (value.endsWith("inf")) {
        value.replace("inf", QString("%1").arg(InfDouble));
    }

    return value.toDouble(&ok);
}

QString MatisseParameters::getStringParamValue(QString paramGroupName, QString paramName)
{
    QString value = _hashValues.value(paramGroupName,QHash<QString,QString>()).value(paramName,"");

    return value;
}

QMatrix3x3 MatisseParameters::getMatrix3x3ParamValue(QString paramGroupName, QString paramName, bool &ok)
{
    QMatrix3x3 values;

    QString valuesStr = _hashValues.value(paramGroupName,QHash<QString,QString>()).value(paramName,"");

    qDebug() << "valuesStr = " << valuesStr;

    QStringList args = valuesStr.split(";");
    if (args.size() != 9) {
        ok = false;
    } else {
        int index = 0;
        bool locOk;
        ok = true;
        for (int noRow = 0; noRow < 3; noRow++) {
            for (int noCol = 0; noCol < 3; noCol++) {
                QString arg = args.at(index).trimmed();
                if (arg.endsWith("inf")) {
                    arg.replace("inf", QString("%1").arg(InfDouble));
                }

                values(noRow, noCol) = arg.toDouble(&locOk);
                ok = ok && locOk;
                index++;
            }
        }
     }

    return values;
}

Matrix6x1 MatisseParameters::getMatrix6x1ParamValue(QString paramGroupName, QString paramName, bool &ok)
{
    Matrix6x1 values;
    QString valuesStr = _hashValues.value(paramGroupName,QHash<QString,QString>()).value(paramName,"");

    QStringList args = valuesStr.split(";");
    if (args.size() != 6) {
        ok = false;
    } else {
        int index = 0;
        bool locOk;
        ok = true;
        for (int noCol = 0; noCol < 6; noCol++) {
            QString arg = args.at(index).trimmed();
            if (arg.endsWith("inf")) {
                arg.replace("inf", QString("%1").arg(InfDouble));
            }
                values(0, noCol) =arg.toDouble(&locOk);
                ok = ok && locOk;
                index++;
            }
     }

    return values;
}

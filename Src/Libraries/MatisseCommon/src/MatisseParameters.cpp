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
        qCritical() << "Empty file name";
        return false;
    }

    QFile inputFile(xmlFilename);
    pFileInfo = new QFileInfo(inputFile);
    if (!inputFile.exists()) {
        _lastErrorStr = "File " + xmlFilename + " not found.";
        qCritical() << QString("Parameters file '%1' not found").arg(xmlFilename);
        return false;
    }

    if (!inputFile.open(QIODevice::ReadOnly)) {
        _lastErrorStr = "Cannot open file " + xmlFilename + ".";
        qCritical() << QString("Parameters file '%1' could not be opened").arg(xmlFilename);
        return false;
    }

    QXmlStreamReader reader(&inputFile);

    int level = -99; //-99: pas de fichier mosaic 0: avant debut fichier, 1: avant debut struct, 2: avant debut var, 3: avant debut val
    //    bool start = true;
    QString structName;
    QString paramName;
    bool ok = true;
    while (!reader.atEnd()) {
        QXmlStreamReader::TokenType type = reader.readNext();
        QString name = reader.name().toString().trimmed();

        switch(type) {
        case QXmlStreamReader::StartElement:
        {
            if (name == "JobParameters") {
                level = 1;
            } else {
                switch (level) {
                case 1: {
                    // on ajoute une structure...
                    QXmlStreamAttributes attributes = reader.attributes();
                    structName = attributes.value("name").toString();
                    _hashValues.insert(structName,QHash<QString, QString>());
                    level = 2;
                }
                    break;
                case 2: {
                    // on defini un parametre
                    QXmlStreamAttributes attributes = reader.attributes();
                    paramName = attributes.value("name").toString();
                    _hashValues[structName].insert(paramName, ""); // allow empty param to be added
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
                if (!_hashValues[structName].value(paramName, "").isEmpty()) {
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
            level--;

            }
                break;

            case QXmlStreamReader::StartDocument:
            break;

            default: {
                qWarning() << QString("Unknown tag type '%1' found while parsing parameters value").arg(type);
                _lastErrorStr = "XML file not compliant: format error";
                ok = false;
            }
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

bool MatisseParameters::containsParam(QString paramStructName, QString paramName)
{
    if (_hashValues.find(paramStructName) != _hashValues.end()) {
        QHash<QString, QString> params = _hashValues.value(paramStructName);
        if (params.find(paramName) != params.end()) {
            return true;
        }
    }
    return false;

}

qint64 MatisseParameters::getIntParamValue(QString paramStructName, QString paramName, bool &ok)
{
    ok=true;
    QString valueStr = _hashValues.value(paramStructName,QHash<QString,QString>()).value(paramName,"").trimmed();

    qint64 value;
    ok = true;
    if (valueStr == "-inf") {
        value=-1 * InfInt;
    } else if (valueStr == "inf") {
        value=InfInt;
    } else {
        value=valueStr.toLongLong(&ok);
    }
    return value;

}

bool MatisseParameters::getBoolParamValue(QString paramStructName, QString paramName, bool &ok)
{
    QString valueStr = _hashValues.value(paramStructName,QHash<QString,QString>()).value(paramName,"").trimmed().toLower();

    bool retValue = false;

    if (_boolRegExpTrue.exactMatch(valueStr)) {
        retValue = true;
        ok = true;
    } else if (_boolRegExpFalse.exactMatch(valueStr)) {
        ok = true;
    } else {
        ok = false;
    }

    return retValue;
}

double MatisseParameters::getDoubleParamValue(QString paramStructName, QString paramName, bool &ok)
{
    ok=true;
    QString valueStr = _hashValues.value(paramStructName,QHash<QString,QString>()).value(paramName,"").trimmed();
    double value;
    ok = true;
    if (valueStr == "-inf") {
        value=-1 * InfDouble;
    } else if (valueStr == "inf") {
        value=InfDouble;
    } else {
        value=valueStr.toDouble(&ok);
    }
    return value;
}

QString MatisseParameters::getStringParamValue(QString paramStructName, QString paramName)
{
    QString value = _hashValues.value(paramStructName,QHash<QString,QString>()).value(paramName,"");

    return value;
}

CameraInfo MatisseParameters::getCamInfoParamValue(QString paramStructName, QString paramName, bool &ok)
{
    CameraInfo cam_info;

    QString valuesStr = _hashValues.value(paramStructName,QHash<QString,QString>()).value(paramName,"");

    if( cam_info.fromQString(valuesStr) )
        ok=true;
    else
        ok=false;

    return cam_info;
}

QMatrix3x3 MatisseParameters::getMatrix3x3ParamValue(QString paramStructName, QString paramName, bool &ok)
{
    QMatrix3x3 values;

    QString valuesStr = _hashValues.value(paramStructName,QHash<QString,QString>()).value(paramName,"");

    QStringList args = valuesStr.split(";");
    if (args.size() != 9) {
        ok = false;
    } else {
        int index = 0;
        bool locOk;
        ok = true;
        for (int noRow = 0; noRow < 3; noRow++) {
            for (int noCol = 0; noCol < 3; noCol++) {
                QString valueStr = args.at(index).trimmed();

                double value;
                locOk = true;
                if (valueStr == "-inf") {
                    value=-1 * InfDouble;
                } else if (valueStr == "inf") {
                    value=InfDouble;
                } else {
                    value=valueStr.toDouble(&locOk);
                }


                values(noRow, noCol) = value;
                ok = ok && locOk;
                index++;
            }
        }
     }

    return values;
}

Matrix6x1 MatisseParameters::getMatrix6x1ParamValue(QString paramStructName, QString paramName, bool &ok)
{
    Matrix6x1 values;
    QString valuesStr = _hashValues.value(paramStructName,QHash<QString,QString>()).value(paramName,"");

    QStringList args = valuesStr.split(";");
    if (args.size() != 6) {
        ok = false;
    } else {
        int index = 0;
        bool locOk;
        ok = true;
        for (int noCol = 0; noCol < 6; noCol++) {
            QString valueStr = args.at(index).trimmed();
            double value;
            locOk = true;
            if (valueStr == "-inf") {
                value=-1 * InfDouble;
            } else if (valueStr == "inf") {
                value=InfDouble;
            } else {
                value=valueStr.toDouble(&locOk);
            }

            values(0, noCol) =value;
            ok = ok && locOk;
            index++;
        }
     }

    return values;
}

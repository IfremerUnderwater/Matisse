#include "matisse_parameters.h"

namespace matisse {

QRegExp MatisseParameters::m_bool_reg_exp_true("^(true|1)$");
QRegExp MatisseParameters::m_bool_reg_exp_false("^(false|0)$");

MatisseParameters::MatisseParameters(QString _xml_filename)
    : m_p_file_info(NULL)
{
    m_last_error_str = "";
    if (_xml_filename.trimmed() != "") {
        loadFile(_xml_filename);
    }
}

MatisseParameters::~MatisseParameters()
{
    if (m_p_file_info!=NULL)
        delete m_p_file_info;
}

bool MatisseParameters::loadFile(QString _xml_filename)
{
    m_hash_values.clear();

    _xml_filename = _xml_filename.simplified().trimmed();
    if (_xml_filename == "") {
        m_last_error_str = "Xml file not defined";
        qCritical() << "Empty file name";
        return false;
    }

    QFile input_file(_xml_filename);
    m_p_file_info = new QFileInfo(input_file);
    if (!input_file.exists()) {
        m_last_error_str = "File " + _xml_filename + " not found.";
        qCritical() << QString("Parameters file '%1' not found").arg(_xml_filename);
        return false;
    }

    if (!input_file.open(QIODevice::ReadOnly)) {
        m_last_error_str = "Cannot open file " + _xml_filename + ".";
        qCritical() << QString("Parameters file '%1' could not be opened").arg(_xml_filename);
        return false;
    }

    QXmlStreamReader reader(&input_file);

    int level = -99; //-99: pas de fichier mosaic 0: avant debut fichier, 1: avant debut struct, 2: avant debut var, 3: avant debut val
    //    bool start = true;
    QString struct_name;
    QString param_name;
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
                    struct_name = attributes.value("name").toString();
                    m_hash_values.insert(struct_name,QHash<QString, QString>());
                    level = 2;
                }
                    break;
                case 2: {
                    // on defini un parametre
                    QXmlStreamAttributes attributes = reader.attributes();
                    param_name = attributes.value("name").toString();
                    m_hash_values[struct_name].insert(param_name, ""); // allow empty param to be added
                    level = 3;
                }
                    break;
                default: {
                    // on ne doit pas se trouver là...
                    m_last_error_str = "XML file not compliant: start element found where value element expected";
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
                QString old_value = "";
                if (!m_hash_values[struct_name].value(param_name, "").isEmpty()) {
                    old_value = m_hash_values[struct_name].value(param_name, "") + ", ";
                }
                m_hash_values[struct_name].insert(param_name, old_value + value);
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

            case QXmlStreamReader::Comment:
              // ignore (remove warning "Unknown tag type '7'...")
              break;

            default: {
                qWarning() << QString("Unknown tag type '%1' found while parsing parameters value").arg(type);
                m_last_error_str = "XML file not compliant: format error";
                ok = false;
            }
                break;
        }

    }

    input_file.close();

    return ok;
}

QFileInfo *MatisseParameters::getXmlFileInfo()
{
    return m_p_file_info;
}

QString MatisseParameters::lastErrorStr()
{
    return m_last_error_str;
}

QString MatisseParameters::dumpStructures()
{
    QString dump;

    foreach(QString structName, m_hash_values.keys()) {
        dump.append("Structure :" + structName + "\n");
        QHash<QString, QString> params = m_hash_values.value(structName);
        foreach(QString param_name, params.keys()) {
            dump.append("\t\tParam:" + param_name + ": " + params.value(param_name) + "\n");
        }
        dump.append("\n");
    }

    return dump;
}

bool MatisseParameters::containsParam(QString _param_struct_name, QString _param_name)
{
    if (m_hash_values.find(_param_struct_name) != m_hash_values.end()) {
        QHash<QString, QString> params = m_hash_values.value(_param_struct_name);
        if (params.find(_param_name) != params.end()) {
            return true;
        }
    }
    return false;

}

qint64 MatisseParameters::getIntParamValue(QString _param_struct_name, QString _param_name, bool &_ok)
{
    _ok=true;
    QString value_str = m_hash_values.value(_param_struct_name,QHash<QString,QString>()).value(_param_name,"").trimmed();

    qint64 value;
    _ok = true;
    if (value_str == "-inf") {
        value=-1 * InfInt;
    } else if (value_str == "inf") {
        value=InfInt;
    } else {
        value=value_str.toLongLong(&_ok);
    }
    return value;

}

bool MatisseParameters::getBoolParamValue(QString _param_struct_name, QString _param_name, bool &_ok)
{
    QString value_str = m_hash_values.value(_param_struct_name,QHash<QString,QString>()).value(_param_name,"").trimmed().toLower();

    bool ret_value = false;

    if (m_bool_reg_exp_true.exactMatch(value_str)) {
        ret_value = true;
        _ok = true;
    } else if (m_bool_reg_exp_false.exactMatch(value_str)) {
        _ok = true;
    } else {
        _ok = false;
    }

    return ret_value;
}

double MatisseParameters::getDoubleParamValue(QString _param_struct_name, QString _param_name, bool &_ok)
{
    _ok = true;
    QString value_str = m_hash_values.value(_param_struct_name,QHash<QString,QString>()).value(_param_name,"").trimmed();
    double value;
    _ok = true;
    if (value_str == "-inf") {
        value=-1 * InfDouble;
    } else if (value_str == "inf") {
        value=InfDouble;
    } else {
        value=value_str.toDouble(&_ok);
    }
    return value;
}

QString MatisseParameters::getStringParamValue(QString _param_struct_name, QString _param_name)
{
    QString value = m_hash_values.value(_param_struct_name,QHash<QString,QString>()).value(_param_name,"");

    return value;
}

CameraInfo MatisseParameters::getCamInfoParamValue(QString _param_struct_name, QString _param_name, bool &_ok)
{
    CameraInfo cam_info;

    QString values_str = m_hash_values.value(_param_struct_name,QHash<QString,QString>()).value(_param_name,"");

    if( cam_info.fromQString(values_str) )
        _ok=true;
    else
        _ok=false;

    return cam_info;
}

QMatrix3x3 MatisseParameters::getMatrix3x3ParamValue(QString _param_struct_name, QString _param_name, bool &_ok)
{
    QMatrix3x3 values;

    QString values_str = m_hash_values.value(_param_struct_name,QHash<QString,QString>()).value(_param_name,"");

    QStringList args = values_str.split(";");
    if (args.size() != 9) {
        _ok = false;
    } else {
        int index = 0;
        bool loc_ok;
        _ok = true;
        for (int no_row = 0; no_row < 3; no_row++) {
            for (int no_col = 0; no_col < 3; no_col++) {
                QString value_str = args.at(index).trimmed();

                double value;
                loc_ok = true;
                if (value_str == "-inf") {
                    value=-1 * InfDouble;
                } else if (value_str == "inf") {
                    value=InfDouble;
                } else {
                    value=value_str.toDouble(&loc_ok);
                }


                values(no_row, no_col) = value;
                _ok = _ok && loc_ok;
                index++;
            }
        }
     }

    return values;
}

Matrix6x1 MatisseParameters::getMatrix6x1ParamValue(QString _param_struct_name, QString _param_name, bool &_ok)
{
    Matrix6x1 values;
    QString values_str = m_hash_values.value(_param_struct_name,QHash<QString,QString>()).value(_param_name,"");

    QStringList args = values_str.split(";");
    if (args.size() != 6) {
        _ok = false;
    } else {
        int index = 0;
        bool loc_ok;
        _ok = true;
        for (int no_col = 0; no_col < 6; no_col++) {
            QString value_str = args.at(index).trimmed();
            double value;
            loc_ok = true;
            if (value_str == "-inf") {
                value=-1 * InfDouble;
            } else if (value_str == "inf") {
                value=InfDouble;
            } else {
                value=value_str.toDouble(&loc_ok);
            }

            values(0, no_col) =value;
            _ok = _ok && loc_ok;
            index++;
        }
     }

    return values;
}

void MatisseParameters::substituteParamValue(QString _source_structure, QString _source_param, QString _target_structure, QString _target_param) {
    if (!m_hash_values.contains(_target_structure)) {
        qCritical() << QString("Trying to substitue parameter value in unkbown structure '%1'").arg(_target_structure);
        return;
    }

    if (!m_hash_values[_source_structure].contains(_target_param)) {
        qCritical() << QString("Trying to substitue value for unknown parameter '%1'").arg(_target_param);
        return;
    }

    if (!m_hash_values.contains(_source_structure)) {
        qCritical() << QString("Trying to substitue parameter value from unknown source structure '%1'").arg(_source_structure);
        return;
    }

    if (!m_hash_values[_source_structure].contains(_source_param)) {
        qCritical() << QString("Trying to substitue parameter value from unknown source parameter '%1'").arg(_source_param);
        return;
    }

    QString source_value = m_hash_values[_source_structure].value(_source_param);

    qDebug() << QString("Substituting value for parameter %1/%2 : %3").arg(_target_structure).arg(_target_param).arg(source_value);
    m_hash_values[_target_structure].insert(_target_param, source_value);
}

} // namespace matisse

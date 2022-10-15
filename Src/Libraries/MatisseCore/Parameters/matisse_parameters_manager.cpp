#include "matisse_parameters_manager.h"
#include "graphical_charter.h"

namespace matisse {

QMap<QString, eParameterType> MatisseParametersManager::m_enum_types;
QMap<QString, eParameterLevel> MatisseParametersManager::m_enum_levels;
QMap<QString, eParameterShow> MatisseParametersManager::m_enum_shows;

QSet<QString> MatisseParametersManager::m_dataset_param_names;
QSet<QString> MatisseParametersManager::m_remote_dataset_param_names;

QRegExp MatisseParametersManager::m_interval_range_expr("^(\\[|\\])((\\-?\\d+((\\.)\\d+)?)|(-inf)),((\\-?\\d+((\\.)\\d+)?)|(\\+?inf))(\\[|\\])$");
QRegExp MatisseParametersManager::m_set_range_expr("^\\{\\w+(,\\w+)*\\}$");

QRegExp MatisseParametersManager::m_matrix_expr("^\\(\\d+(,\\d+)?\\)$");
QRegExp MatisseParametersManager::m_matrix_values_expr("^\\-?\\d+((\\.)\\d+)?(;\\-?\\d+((\\.)\\d+)?)*$");

double MatisseParametersManager::m_epsilon = 0.000001;
QString MatisseParametersManager::m_inf_str = QString::number(quint64(InfInt));


MatisseParametersManager::MatisseParametersManager(QObject *_parent) :
    QObject(_parent),
    m_dictionnary_labels(),
    m_structures(),
    m_selected_assembly(""),
    m_groups_widgets(),
    m_values_widgets(),
    m_dialogs(),
    m_is_read_only_mode(false)
{
    loadStaticCollections();
    m_dictionnary_labels.declareLabels();
}

void MatisseParametersManager::loadStaticCollections()
{
    if (!m_enum_types.isEmpty()) {
        // static collections already initialized
        return;
    }

    //_enumTypes.clear();
    //_enumTypes.insert("unknown", UNKNOWN_TYPE);
    m_enum_types.insert("integer", PAR_INT);
    m_enum_types.insert("float", PAR_FLOAT);
    m_enum_types.insert("double", PAR_DOUBLE);
    m_enum_types.insert("string", PAR_STRING);
    m_enum_types.insert("bool", PAR_BOOL);
    m_enum_types.insert("camera", PAR_CAMERA);

    //_enumShows.clear();
    //_enumShows.insert("unknown", UNKNOWN_SHOW);
    m_enum_shows.insert("line", LINE_EDIT);
    m_enum_shows.insert("combo", COMBO_BOX);
    m_enum_shows.insert("cam_combo", CAM_COMBO_BOX);
    m_enum_shows.insert("list", LIST_BOX);
    m_enum_shows.insert("spin", SPIN_BOX);
    m_enum_shows.insert("spinDouble", DOUBLE_SPIN_BOX);
    m_enum_shows.insert("check", CHECK_BOX);
    m_enum_shows.insert("table", TABLE);
    m_enum_shows.insert("file", FILE_SELECTOR_ABSOLUTE);
    //    _enumShows.insert("file:relative", FILE_SELECTOR_RELATIVE);
    //    _enumShows.insert("file:absolute", FILE_SELECTOR_ABSOLUTE);
    m_enum_shows.insert("dir", DIR_SELECTOR_ABSOLUTE);
    //    _enumShows.insert("dir:relative", DIR_SELECTOR_RELATIVE);
    //    _enumShows.insert("dir:absolute", DIR_SELECTOR_ABSOLUTE);

    m_enum_levels.insert("user", USER);
    m_enum_levels.insert("advanced", ADVANCED);
    m_enum_levels.insert("expert", EXPERT);

    m_dataset_param_names.insert(DATASET_PARAM_OUTPUT_DIR);
    m_dataset_param_names.insert(DATASET_PARAM_OUTPUT_FILENAME);
    m_dataset_param_names.insert(DATASET_PARAM_DATASET_DIR);
    m_dataset_param_names.insert(DATASET_PARAM_NAVIGATION_FILE);

    m_remote_dataset_param_names.insert(DATASET_PARAM_REMOTE_OUTPUT_DIR);
    m_remote_dataset_param_names.insert(DATASET_PARAM_REMOTE_OUTPUT_FILENAME);
    m_remote_dataset_param_names.insert(DATASET_PARAM_REMOTE_DATASET_DIR);
    m_remote_dataset_param_names.insert(DATASET_PARAM_REMOTE_NAVIGATION_FILE);
    m_remote_dataset_param_names.insert(DATASET_PARAM_REMOTE_DATASET_PARENT_DIR);
}

void MatisseParametersManager::checkDictionnaryComplete()
{
    if (!m_structures.contains(DATASET_STRUCTURE)) {
        qFatal("Dictionnary does not contain mandatory 'dataset_param' structure");
    }

    Structure dataset_struct = m_structures.value(DATASET_STRUCTURE);
    QList<ParametersGroup> dataset_groups = dataset_struct.parameters_groups;

    QSet<QString> found_dataset_params;

    foreach (ParametersGroup group, dataset_groups) {
        QStringList params = group.parameters_names;

        foreach (QString param, params) {
            if (m_dataset_param_names.contains(param)) {
                found_dataset_params.insert(param);
            }
        }
    }

    if (found_dataset_params != m_dataset_param_names) {

        QStringList found = found_dataset_params.values();
        QStringList expected = m_dataset_param_names.values();
        QString full_message = QString("Dataset parameters were not all found in the dictionnary\nExpected: ")
                .append(expected.join(",")).append("\nFound: ").append(found.join(","));
#if (QT_VERSION >= QT_VERSION_CHECK(5, 0, 0))
        qFatal("%s\n",full_message.toLatin1().constData());
#else
        qFatal("%s\n",full_message.toAscii().constData());
#endif
    }
}

bool MatisseParametersManager::readDictionnaryFile(QString _xml_filename)
{
    m_structures.clear();

    QFile input_file(_xml_filename);
    if (!input_file.exists()) {
        qDebug() << "File not found..." << _xml_filename;
        return false;
    }

    if (!input_file.open(QIODevice::ReadOnly)) {
        qDebug() << "File not opened...";
        return false;
    }

    QXmlStreamReader reader(&input_file);
    QXmlStreamAttributes attributes;

    QString structure_name;
    QString group_name;
    QString group_text = "";
    QString enums_name;
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
                m_dico_publication_timestamp = QDateTime::fromString(attributes.value("publicationTimestamp").toString(), "yyyy-MM-dd'T'hh:mm:ss");
            } else if (name == "Structure") {
                structure_name = attributes.value("name" ).toString();
                // qDebug() << "Trouvé structure" << structureName;
                Structure new_struct;
                new_struct.name = structure_name;
                new_struct.has_user_values = false;

                /* check structure name unicity (not supported by Qt XML schema validation) */
                if (m_structures.contains(structure_name)) {
                    qFatal("%s\n",QString("Unique constraint violation for structure name '%1'").arg(structure_name).toStdString().c_str());
                }
                m_structures.insert(structure_name, new_struct);
                m_structures_names << structure_name;
            } else if (name == "ParametersGroup") {
                group_name = attributes.value("name").toString();
                group_text = tr(attributes.value("text").toString().toLatin1());
            } else if (name == "Parameter") {
                addParameter(structure_name, group_name, group_text, attributes);
            } else if (name == "Enum") {
                enums_name = attributes.value("name" ).toString();
                Enums new_enums;
                new_enums.name = enums_name;
                m_enums.insert(enums_name, new_enums);
            } else if (name == "EnumValue") {
                addEnum(enums_name, attributes);
            }
        }

        if (reader.hasError()) {
            qWarning() << "An error occurred while reading the dictionnary file:\n" << reader.errorString();
        }
    }

    reader.clear();
    input_file.close();

    // check dictionnary integrity
    checkDictionnaryComplete();

    return (m_structures.size() > 0);
}

bool MatisseParametersManager::addUserModuleForParameter(QString _user_module, QString _structure_name, QString _param_name)
{
    if (!m_structures.contains(_structure_name)) {
        qCritical() << "Structure not found in parameters dictionnary : " << _structure_name;
        return false;
    }

    if (!m_structure_by_parameter.contains(_param_name)) {
        qCritical() << "Parameter not found in dictionnary : " << _param_name;
        return false;
    }

    QString dico_struct_name = m_structure_by_parameter.value(_param_name);
    if (dico_struct_name != _structure_name) {
        qWarning() << QString("Parameter '%1' found in dictionnary in structure '%2', not '%3'").arg(_param_name).arg(dico_struct_name).arg(_structure_name);
    }

    if (m_dataset_param_names.contains(_param_name)) {
        qWarning() << QString("Trying to add dataset parameter '%1' as expected parameter. It will be ignored").arg(_param_name);
        return false;
    }

    if (!m_expected_parameters.contains(_param_name)) {
        qDebug() << "Adding expected parameter " << dico_struct_name << _param_name;
        QSet<QString> *param_users = new QSet<QString>();
        param_users->insert(_user_module);
        m_expected_parameters.insert(_param_name, param_users);
    } else {
        qDebug() << "Adding user" << _user_module << "for param" << dico_struct_name << _param_name;
        QSet<QString> *param_users = m_expected_parameters.value(_param_name);
        if (param_users->contains(_user_module)) {
            qWarning() << QString("Module '%1' is already referenced as using param").arg(_user_module) << dico_struct_name << _param_name;
        } else {
            param_users->insert(_user_module);
        }
    }

    // Not necessary to check the presence of the key (already checked for the structure)
    QString group_name = m_group_by_parameter.value(_param_name);

    if (!m_expected_groups.contains(group_name)) {
        qDebug() << "Adding expected group" << group_name;
        QSet<QString> *group_members = new QSet<QString>();
        group_members->insert(_param_name);
        m_expected_groups.insert(group_name, group_members);
    } else {
        qDebug() << "Adding param" << _param_name << "as member of expected group" << group_name;
        QSet<QString> *group_members = m_expected_groups.value(group_name);
        if (group_members->contains(_param_name)) {
            qDebug() << QString("Param '%1' is already referenced as member of expected group '%2'").arg(_param_name).arg(group_name);
        } else {
            group_members->insert(_param_name);
        }
    }

    if (!m_values_widgets.contains(dico_struct_name)) {
        qCritical() << "Widgets hash not found for structure " << dico_struct_name;
        return false;
    }

    QMap<QString, EnrichedFormWidget*> structure_params_widgets = m_values_widgets.value(dico_struct_name);
    QMap<QString, QWidget*> structure_groups_widgets = m_groups_widgets.value(dico_struct_name);

    if (!structure_params_widgets.contains(_param_name)) {
        qCritical() << QString("Widgets hash for structure '%1' does not contain widget for parameter '%2'").arg(dico_struct_name).arg(_param_name);
        return false;
    }

    EnrichedFormWidget* param_widget = structure_params_widgets.value(_param_name);
    QWidget* group_widget = structure_groups_widgets.value(_param_name);
    if (!param_widget) {
        qCritical() << "Widgets for parameter" << _param_name << "is null";
        return false;
    }

    group_widget->show();
    param_widget->show();

    return true;
}

bool MatisseParametersManager::removeUserModuleForParameter(QString _user_module, QString _structure_name, QString _param_name)
{
    if (!m_structures.contains(_structure_name)) {
        qCritical() << "Structure not found in parameters dictionnary : " << _structure_name;
        return false;
    }

    if (!m_structure_by_parameter.contains(_param_name)) {
        qCritical() << "Parameter not found in dictionnary : " << _param_name;
        return false;
    }

    QString dico_struct_name = m_structure_by_parameter.value(_param_name);
    if (dico_struct_name != _structure_name) {
        qWarning() << QString("Parameter '%1' found in dictionnary in structure '%2', not '%3'").arg(_param_name).arg(dico_struct_name).arg(_structure_name);
    }

    if (!m_expected_parameters.contains(_param_name)) {
        qCritical() << QString("Parameter '%1' not referenced as expected parameter");
        return false;
    }

    QSet<QString> *users = m_expected_parameters.value(_param_name);
    if (!users->contains(_user_module)) {
        qCritical() << QString("User module '%1' not referenced as expecting parameter '%2'").arg(_user_module).arg(_param_name);
        return false;
    }

    if (!m_group_by_parameter.contains(_param_name)) {
        qCritical() << QString("No group found for parameter '%1'").arg(_param_name);
        return false;
    }

    QString group_name = m_group_by_parameter.value(_param_name);

    if (!m_expected_groups.contains(group_name)) {
        qCritical() << QString("No members list found for group '%1'").arg(group_name);
        return false;
    }

    QSet<QString> *group_members = m_expected_groups.value(group_name);
    if (!group_members->contains(_param_name)) {
        qCritical() << QString("Parameter '%1' not found in members list for group '%2'").arg(_param_name).arg(group_name);
        return false;
    }

    qDebug() << QString("Removing module '%1' as expecting parameter '%2'").arg(_user_module).arg(_param_name);
    users->remove(_user_module);

    if (users->isEmpty()) {
        qDebug() << QString("Parameter '%1' is no longer expected, hiding field").arg(_param_name);
        m_expected_parameters.remove(_param_name);
        delete users;

        // de-reference parameter for group members
        group_members->remove(_param_name);

        bool hide_group = false;

        // de-reference group if does not contain any expected parameter
        if (group_members->isEmpty()) {
            m_expected_groups.remove(group_name);
            delete group_members;
            hide_group = true;
        } else {
            // see if group still has other parameters of same level
            hide_group = true;

            Parameter parameter = m_parameters.value(_param_name);

            foreach (QString other_param_name, *group_members) {
                Parameter other_parameter = m_parameters.value(other_param_name);
                if (other_parameter.level == parameter.level) {
                    hide_group = false;
                    break;
                }
            }
        }

        /* handling parameters and groups hiding if necessary */
        QMap<QString, EnrichedFormWidget*> structure_params_widgets = m_values_widgets.value(dico_struct_name);
        QMap<QString, QWidget*> structure_groups_widgets = m_groups_widgets.value(dico_struct_name);

        if (!structure_params_widgets.contains(_param_name)) {
            qCritical() << QString("Widgets hash for structure '%1' does not contain widget for parameter '%2'").arg(dico_struct_name).arg(_param_name);
            return false;
        }

        EnrichedFormWidget* param_widget = structure_params_widgets.value(_param_name);
        QWidget* group_widget = structure_groups_widgets.value(_param_name);
        if (!param_widget) {
            qCritical() << "Widgets for parameter" << _param_name << "is null";
            return false;
        }

        param_widget->hide();
        if (hide_group) {
            group_widget->hide();
        }
    }

    return true;
}

bool MatisseParametersManager::clearExpectedParameters()
{
    QSet<QWidget*> group_widgets;

    // clear expected parameters and using modules
    QList<QString> expected_param_names = m_expected_parameters.keys();

    QList<QString> params_to_hide(expected_param_names);

    /* Add dataset parameter group(s) in the list to hide */
    foreach (QString dataset_param_name, m_dataset_param_names) {
        params_to_hide.append(dataset_param_name);
    }

    foreach (QString param, params_to_hide) {
        /* dataset parameters are not expected */
        if (m_expected_parameters.contains(param)) {
            QSet<QString> *users = m_expected_parameters.value(param);
            m_expected_parameters.remove(param);
            users->clear();
            delete users;
        }

        QString structure = m_structure_by_parameter.value(param);
        QWidget* param_widget = m_values_widgets.value(structure).value(param);
        param_widget->hide();

        QWidget* group_widget = m_groups_widgets.value(structure).value(param);
        if (!group_widgets.contains(group_widget)) {
            group_widgets.insert(group_widget);
        }
    }

    // clear expected groups and member parameters
    QList<QString> groups = m_expected_groups.keys();
    foreach (QString group, groups) {
        /* dataset group is not expected */
        if (m_expected_groups.contains(group)) {
            QSet<QString> *members = m_expected_groups.value(group);
            m_expected_groups.remove(group);
            members->clear();
            delete members;
        }
    }

    foreach (QWidget* group_widget, group_widgets) {
        group_widget->hide();
    }
    group_widgets.clear();

    return true;
}

bool MatisseParametersManager::addParameter(QString _struct_name, QString _group_name, QString _group_text, QXmlStreamAttributes _attributes)
{
    if (!m_structures.contains(_struct_name)) {
        qCritical() << QString("Structure '%1'' not found, cannot add parameter").arg(_struct_name);
        return false;
    }

    // searching group. If it doesn't exist we create it.
    int noGroup = m_structures[_struct_name].groups_names.indexOf(_group_name);
    if (noGroup == -1) {
        m_structures[_struct_name].groups_names.append(_group_name);
        ParametersGroup new_group;
        new_group.name = _group_name;
        new_group.text = _group_text;
        new_group.has_user_values = false;
        m_structures[_struct_name].parameters_groups.append(new_group);
        noGroup = m_structures[_struct_name].groups_names.size() - 1;

        /* check unique constraint for group name */
        if (m_groups.contains(_group_name)) {
            qFatal("%s\n",QString("Unique constraint violation for group name '%1'").arg(_group_name).toStdString().c_str());
        }

        m_groups.insert(_group_name, new_group);
    }

    Parameter parameter;

    parameter.name = _attributes.value("name").toString();

    QString level_str = _attributes.value("level").toString().toLower();
    eParameterLevel level = m_enum_levels.value(level_str);
    parameter.level = level;

    parameter.text = tr(_attributes.value("text").toLatin1());

    QString type_str = _attributes.value("type").toString().toLower();
    parameter.type = m_enum_types.value(type_str);

    QString show_str = _attributes.value("show").toString().toLower();
    parameter.show = m_enum_shows.value(show_str);

    // double spin...
    if (((parameter.type == PAR_DOUBLE) || (parameter.type == PAR_FLOAT)) && (parameter.show == SPIN_BOX)) {
        parameter.show = m_enum_shows.value("spinDouble");
    }

    parameter.parameter_size = QSize(1,1);
    QString size_str = _attributes.value("size").toString().simplified().replace(" ","");
    if (m_matrix_expr.exactMatch(size_str)) {
        size_str = size_str.mid(1, size_str.size()-2);
        parameter.parameter_size.setWidth(size_str.split(",").at(0).toInt());
        parameter.parameter_size.setHeight(size_str.split(",").at(1).toInt());
    }

    quint8 precision_default = PRECISION_DEFAULT;

    parameter.precision = precision_default; /* default is 2 decimal digits */
    if (_attributes.hasAttribute("precision")) {
        quint8 precision = QString(_attributes.value("precision").toLatin1()).toUShort();
        quint8 precision_max = PRECISION_MAX;
        if (precision > precision_max) {
            qWarning() << QString("Parameter '%1' : precision %2 exceeds max precision %3. Leaving default precision %4'").arg(parameter.name).arg(precision).arg(precision_max).arg(precision_default);
        } else {
            parameter.precision = precision;
        }
    }

    if (_attributes.hasAttribute("formatTemplate")) {
        parameter.format_template = _attributes.value("formatTemplate").toString();
    } else {
        parameter.format_template = "";
    }

    parameter.range = _attributes.value("range").toString().simplified().replace(" ","");

    parameter.value = _attributes.value("default").toString();

    QString param_name = parameter.name;

    m_structures[_struct_name].parameters_groups[noGroup].parameters_names << param_name;
    m_structures[_struct_name].parameters_groups[noGroup].parameters.append(parameter);

    /* check unique constraint for parameter name */
    if (m_parameters.contains(param_name)) {
        qFatal("%s\n",QString("Unique constraint violation for parameter name '%1'").arg(param_name).toStdString().c_str());
    }
    m_parameters.insert(param_name, parameter);

    m_structure_by_parameter.insert(param_name, _struct_name);

    m_group_by_parameter.insert(param_name, _group_name);

    /* Appending parameter name to the usage level list */
    QList<QString>* params_for_level;

    if (!m_parameters_by_level.contains(level)) {
        params_for_level = new QList<QString>;
        m_parameters_by_level.insert(level, params_for_level);
    } else {
        params_for_level = m_parameters_by_level.value(level);
    }

    params_for_level->append(param_name);

    return true;

}

bool MatisseParametersManager::addEnum(QString _enums_name, QXmlStreamAttributes _attributes)
{
    if (!m_enums.contains(_enums_name)) {
        return false;
    }
    eEnumValue enum_value;

    enum_value.name = _attributes.value("name").toString();
    enum_value.text = _attributes.value("text").toString();

    m_enums[_enums_name].values.append(enum_value);

    return true;

}

ParametersWidgetSkeleton *MatisseParametersManager::generateParametersWidget(QWidget *_owner)
{
    qDebug() << "Building parameters widget";
    m_full_parameters_widget = new ParametersWidgetSkeleton(_owner);
    m_full_parameters_widget->setObjectName("_WID_parametersWidget");

    generateLevelParametersWidget(USER);
    generateLevelParametersWidget(ADVANCED);
    generateLevelParametersWidget(EXPERT);

    translateHeaderButtons();

    connect(m_full_parameters_widget, SIGNAL(si_translateParameters()), this, SLOT(sl_translateParameters()));

    applyApplicationContext(false, false);

    return m_full_parameters_widget;
}

void MatisseParametersManager::generateLevelParametersWidget(eParameterLevel _level)
{
    GraphicalCharter &graph_chart = GraphicalCharter::instance();

    ParametersHeaderButton *level_header_button = new ParametersHeaderButton(m_full_parameters_widget, _level);
    level_header_button->setObjectName("_PB_levelHeaderButton");
    m_full_parameters_widget->addWidget(level_header_button);

    m_header_buttons_by_level.insert(_level, level_header_button);

    connect(level_header_button, SIGNAL(clicked(bool)), this, SLOT(sl_foldUnfoldLevelParameters()));

    QWidget *level_container = new QWidget(m_full_parameters_widget);
    level_container->setObjectName("_WID_levelParametersContainer");
    QVBoxLayout *level_container_layout = new QVBoxLayout();
    level_container->setLayout(level_container_layout);
    m_full_parameters_widget->addWidget(level_container);

    m_param_containers_by_level.insert(_level, level_container);

    QList<QString>* level_params_list = m_parameters_by_level.value(_level);

    QSet<QString> level_groups;

    QGroupBox* current_group = NULL;

    foreach (QString param_name, *level_params_list) {
        QString struct_name = m_structure_by_parameter.value(param_name);
        QString group_name = m_group_by_parameter.value(param_name);

        ParametersGroup group;
        QList<ParametersGroup> groups = m_structures[struct_name].parameters_groups;
        foreach (ParametersGroup g, groups) {
            if (g.name == group_name) {
                group = g;
                break;
            }
        }

        if (!level_groups.contains(group_name)) {
            /* Hide previous group */
            if (current_group) {
                current_group->hide();
            }

            QString groupLabel = m_dictionnary_labels.getGroupLabel(group_name);
            //currentGroup = new QGroupBox(group._text, _fullParametersWidget);
            current_group = new QGroupBox(groupLabel, m_full_parameters_widget);
            QVBoxLayout* currentGroupLayout = new QVBoxLayout();
            currentGroupLayout->setContentsMargins(0, graph_chart.dpiScaled(PARAM_GROUP_MARGIN_TOP), 0, graph_chart.dpiScaled(PARAM_GROUP_MARGIN_BOTTOM));

            current_group->setLayout(currentGroupLayout);
            level_container_layout->addWidget(current_group);

            level_groups.insert(group_name);

            /* referencing group for deferred translation */
            QMap<QString, QGroupBox*> *level_group_widgets;
            if (!m_group_widgets_by_level.contains(_level)) {
                level_group_widgets = new QMap<QString, QGroupBox*>();
                m_group_widgets_by_level.insert(_level, level_group_widgets);
            } else {
                level_group_widgets = m_group_widgets_by_level.value(_level);
            }

            level_group_widgets->insert(group_name, current_group);
        }

        Parameter param;
        QList<Parameter> params = group.parameters;
        foreach (Parameter p, params) {
            if (p.name == param_name) {
                param = p;
                break;
            }
        }

        EnrichedFormWidget * widget = NULL;
        QString param_label_text = m_dictionnary_labels.getParamLabel(param_name);

        switch(param.show) {
        case LINE_EDIT: {
            widget = new EnrichedLineEdit(m_full_parameters_widget, param_label_text, param.value.toString());
        }
            break;
        case SPIN_BOX: {
            QVariant min_value;
            QVariant max_value;
            getRange(param, min_value, max_value);
            widget = new EnrichedSpinBox(m_full_parameters_widget, param_label_text, min_value.toString(), max_value.toString(), param.value.toString());
        }
            break;
        case DOUBLE_SPIN_BOX: {
            QVariant min_value;
            QVariant max_value;
            getRange(param, min_value, max_value);
            EnrichedDecimalValueWidget* decimal_value_widget = new EnrichedDoubleSpinBox(m_full_parameters_widget, param_label_text, min_value.toString(), max_value.toString(), param.value.toString());
            decimal_value_widget->setPrecision(param.precision);
            widget = decimal_value_widget;
        }
            break;
        case COMBO_BOX: {
            QStringList items = getEnums(param);
            widget = new EnrichedComboBox(m_full_parameters_widget, param_label_text, items, param.value.toString());
        }
            break;
        case CAM_COMBO_BOX: {
            widget = new EnrichedCamComboBox(m_full_parameters_widget, param_label_text, param.value.toString());
        }
            break;
        case LIST_BOX: {
            QStringList items = getEnums(param);
            widget = new EnrichedListBox(m_full_parameters_widget, param_label_text, items, param.value.toString());
        }
            break;
        case CHECK_BOX: {
            widget = new EnrichedCheckBox(m_full_parameters_widget, param_label_text, getBoolValue(param.value));
        }
            break;
        case TABLE: {
            int nb_cols = param.parameter_size.width();
            int nb_rows = param.parameter_size.height();
            QStringList values = getNumList(param);
            EnrichedDecimalValueWidget* decimal_value_widget = new EnrichedTableWidget(m_full_parameters_widget, param_label_text, nb_cols, nb_rows, values, param.format_template);
            decimal_value_widget->setPrecision(param.precision);
            widget = decimal_value_widget;
        }
            break;
        case FILE_SELECTOR_RELATIVE:
        case FILE_SELECTOR_ABSOLUTE: {
            widget = new EnrichedFileChooser(m_full_parameters_widget, m_icon_factory, param_label_text, param.show, param.value.toString());
        }
            break;
        case DIR_SELECTOR_RELATIVE:
        case DIR_SELECTOR_ABSOLUTE: {
            widget = new EnrichedFileChooser(m_full_parameters_widget, m_icon_factory, param_label_text, param.show, param.value.toString());
        }
            break;
        case UNKNOWN_SHOW:
        default:
            break;
        }
        if (widget) {
            widget->setObjectName("_WID_singleParameterForm");
            widget->hide();

            connect(widget, SIGNAL(si_valueChanged(bool)), m_full_parameters_widget, SLOT(sl_valueModified(bool)));

            if (!m_values_widgets.contains(struct_name)) {
                QMap<QString, EnrichedFormWidget*> *struct_widgets = new QMap<QString, EnrichedFormWidget*>();
                m_values_widgets.insert(struct_name, *struct_widgets);
            }
            m_values_widgets[struct_name].insert(param.name, widget);
            m_value_widgets_by_param_name.insert(param.name, widget);

            if (!m_groups_widgets.contains(struct_name)) {
                QMap<QString, QWidget*> *struct_groups_widgets = new QMap<QString, QWidget*>();
                m_groups_widgets.insert(struct_name, *struct_groups_widgets);
            }
            m_groups_widgets[struct_name].insert(param.name, current_group);

            current_group->layout()->addWidget(widget);
        }

    }

    /* hide last group */
    if (current_group) {
        current_group->hide();
    }

}

void MatisseParametersManager::translateHeaderButtons()
{
    ParametersHeaderButton *user_header_button = m_header_buttons_by_level.value(USER);
    user_header_button->setText(tr("User parameters"));

    ParametersHeaderButton *advanced_header_button = m_header_buttons_by_level.value(ADVANCED);
    advanced_header_button->setText(tr("Advanced parameters"));

    ParametersHeaderButton *expert_header_button = m_header_buttons_by_level.value(EXPERT);
    expert_header_button->setText(tr("Expert parameters"));
}

void MatisseParametersManager::sl_translateParameters()
{
    translateHeaderButtons();

    QList<QString> param_names = m_value_widgets_by_param_name.keys();
    foreach (QString param_name, param_names) {
        EnrichedFormWidget *value_widget = m_value_widgets_by_param_name.value(param_name);
        QString translated_label = m_dictionnary_labels.getParamLabel(param_name);
        value_widget->setLabelText(translated_label);
    }

    retranslateLevelGroups(USER);
    retranslateLevelGroups(ADVANCED);
    retranslateLevelGroups(EXPERT);
}

void MatisseParametersManager::sl_foldUnfoldLevelParameters()
{
    ParametersHeaderButton* header_button = qobject_cast<ParametersHeaderButton*>(sender());
    eParameterLevel level = header_button->getLevel();

    QWidget* param_container = m_param_containers_by_level.value(level);

    if (header_button->getIsUnfolded()) {
        param_container->show();
    } else {
        param_container->hide();
    }
}

void MatisseParametersManager::retranslateLevelGroups(eParameterLevel _level)
{
    QMap<QString, QGroupBox*> *level_groups = m_group_widgets_by_level.value(_level);

    QList<QString> group_names = level_groups->keys();
    foreach (QString group_name, group_names) {
        QGroupBox *group_widget = level_groups->value(group_name);
        QString translated_label = m_dictionnary_labels.getGroupLabel(group_name);
        group_widget->setTitle(translated_label);
    }

}
void MatisseParametersManager::applyApplicationContext(bool _is_expert, bool _is_programming)
{
    ParametersHeaderButton *advanced_params_header = m_header_buttons_by_level.value(ADVANCED);
    QWidget *advanced_params_container = m_param_containers_by_level.value(ADVANCED);

    ParametersHeaderButton *expert_params_header = m_header_buttons_by_level.value(EXPERT);
    QWidget *expert_params_container = m_param_containers_by_level.value(EXPERT);

    if (_is_expert) {
        advanced_params_header->setIsUnfolded(_is_programming);
        expert_params_header->setIsUnfolded(_is_programming);
        expert_params_header->show();

        if (_is_programming) {
            advanced_params_container->show();
            expert_params_container->show();
        } else {
            advanced_params_container->hide();
            expert_params_container->hide();
        }

    } else {
        advanced_params_header->setIsUnfolded(false);
        advanced_params_container->hide();
        expert_params_header->hide();
        expert_params_container->hide();
    }
}

void MatisseParametersManager::toggleReadOnlyMode(bool _is_read_only)
{
    if (_is_read_only == m_is_read_only_mode) {
        return;
    }

    qDebug() << "Toggling parameter widgets to mode " << ((_is_read_only) ? "Read-Only" : "Editing");

    foreach (QString struct_name, m_values_widgets.keys()) {
        QMap<QString, EnrichedFormWidget*> structParams = m_values_widgets.value(struct_name);
        foreach (EnrichedFormWidget* widget, structParams.values()) {
            widget->setEnabled(!_is_read_only);
        }
    }

    m_is_read_only_mode = _is_read_only;
}

void MatisseParametersManager::pullDatasetParameters(KeyValueList &_kvl)
{
    foreach (QString dataset_param_name, m_dataset_param_names) {
        if (_kvl.getKeys().contains(dataset_param_name)) {

            /* Get assembly template parameter value */
            QString value = getValue(DATASET_STRUCTURE, dataset_param_name);

            /* Default value is not overriden if the parameter is not defined for the assembly */
            if (value != "") {
                _kvl.set(dataset_param_name, value);
            }
        }
    }
}

void MatisseParametersManager::pullRemoteDatasetParameters(KeyValueList &_kvl)
{
    foreach (QString remote_dataset_param_name, m_remote_dataset_param_names) {
        if (_kvl.getKeys().contains(remote_dataset_param_name)) {

            /* Get assembly template parameter value */
            QString value = getValue(REMOTE_DATASET_STRUCTURE, remote_dataset_param_name);

            /* Default value is not overriden if the parameter is not defined for the assembly */
            if (value != "") {
                _kvl.set(remote_dataset_param_name, value);
            }
        }
    }
}

void MatisseParametersManager::pushPreferredDatasetParameters(KeyValueList _kvl)
{
    m_preferred_dataset_parameters.insert(DATASET_PARAM_OUTPUT_DIR, _kvl.getValue(DATASET_PARAM_OUTPUT_DIR));
    m_preferred_dataset_parameters.insert(DATASET_PARAM_OUTPUT_FILENAME, _kvl.getValue(DATASET_PARAM_OUTPUT_FILENAME));

    foreach (QString pref_param_name, m_preferred_dataset_parameters.keys()) {
        QString pref_param_value = m_preferred_dataset_parameters.value(pref_param_name);

        /* override default value only if the preference value was defined */
        if (pref_param_value == "") {
            continue;
        }

        QWidget* param_widget = m_values_widgets.value(DATASET_STRUCTURE).value(pref_param_name);

        if (!param_widget) {
            qCritical() << QString("Widget for parameter '%1' is null").arg(pref_param_name);
            continue;
        }

        EnrichedFormWidget* actual_param_widget = static_cast<EnrichedFormWidget *>(param_widget);
        actual_param_widget->overrideDefaultValue(pref_param_value);
    }
}

QString MatisseParametersManager::getParameterValue(QString _parameter_name)
{
    QString value;

    if (!m_parameters.contains(_parameter_name)) {
        qCritical() << QString("Parameter '%1' is not defined in dictionnary, cannot retrieve value").arg(_parameter_name);
        return value;
    }

    if (!m_expected_parameters.contains(_parameter_name) && !m_dataset_param_names.contains(_parameter_name)) {
        qCritical() << QString("Parameter '%1' is neither expected nor a dataset parameter, cannot retrieve value").arg(_parameter_name);
        return value;
    }

    QString structure_name = m_structure_by_parameter.value(_parameter_name);
    EnrichedFormWidget *value_widget = m_values_widgets.value(structure_name).value(_parameter_name);
    value = value_widget->currentValue();

    return value;
}

void MatisseParametersManager::pushDatasetParameters(KeyValueList _kvl)
{
    qDebug() << "Push dataset params :\n" << _kvl.getKeys() << "\n" << _kvl.getValues();

    QString result_path = _kvl.getValue(DATASET_PARAM_OUTPUT_DIR);
    QString output_file = _kvl.getValue(DATASET_PARAM_OUTPUT_FILENAME);
    QString data_path = _kvl.getValue(DATASET_PARAM_DATASET_DIR);
    QString navigation_file = _kvl.getValue(DATASET_PARAM_NAVIGATION_FILE);
    QString navigation_source = "";
    if (_kvl.getKeys().contains(DATASET_PARAM_NAVIGATION_SOURCE)) {
        navigation_source = _kvl.getValue(DATASET_PARAM_NAVIGATION_SOURCE);
    }

    m_job_extra_parameters.clear();

    /* update parameter value for output dir */
    EnrichedFormWidget* param_widget;
    param_widget = m_values_widgets.value(DATASET_STRUCTURE).value(DATASET_PARAM_OUTPUT_DIR);
    param_widget->setValue(result_path);

    if (!m_expected_parameters.contains(DATASET_PARAM_OUTPUT_DIR)) {
        qDebug() << "Adding output_dir param to job extra parameters...";
        m_job_extra_parameters.insert(DATASET_PARAM_OUTPUT_DIR);
    }

    /* update parameter value for output file name */
    param_widget = m_values_widgets.value(DATASET_STRUCTURE).value(DATASET_PARAM_OUTPUT_FILENAME);
    param_widget->setValue(output_file);

    if (!m_expected_parameters.contains(DATASET_PARAM_OUTPUT_FILENAME)) {
        m_job_extra_parameters.insert(DATASET_PARAM_OUTPUT_FILENAME);
    }


    /* update parameter value for dataset path */
    param_widget = m_values_widgets.value(DATASET_STRUCTURE).value(DATASET_PARAM_DATASET_DIR);
    param_widget->setValue(data_path);


    if (!m_expected_parameters.contains(DATASET_PARAM_DATASET_DIR)) {
        m_job_extra_parameters.insert(DATASET_PARAM_DATASET_DIR);
    }

    /* update parameter value for navigation file */
    param_widget = m_values_widgets.value(DATASET_STRUCTURE).value(DATASET_PARAM_NAVIGATION_FILE);
    param_widget->setValue(navigation_file);

    if (!m_expected_parameters.contains(DATASET_PARAM_NAVIGATION_FILE)) {
        m_job_extra_parameters.insert(DATASET_PARAM_NAVIGATION_FILE);
    }

    /* update parameter value for navigation source */
    if (!navigation_source.isEmpty()) {
        param_widget = m_values_widgets.value(DATASET_STRUCTURE).value(DATASET_PARAM_NAVIGATION_SOURCE);
        param_widget->setValue(navigation_source);

        if (!m_expected_parameters.contains(DATASET_PARAM_NAVIGATION_SOURCE)) {
            m_job_extra_parameters.insert(DATASET_PARAM_NAVIGATION_SOURCE);
        }
    }
}

void MatisseParametersManager::pushRemoteDatasetParameters(KeyValueList _kvl)
{
    QString remote_dataset_dir = _kvl.getValue(DATASET_PARAM_REMOTE_DATASET_DIR);
    QString remote_navigation_file = _kvl.getValue(DATASET_PARAM_REMOTE_NAVIGATION_FILE);
    QString remote_result_path = _kvl.getValue(DATASET_PARAM_REMOTE_OUTPUT_DIR);
    QString remote_output_file = _kvl.getValue(DATASET_PARAM_REMOTE_OUTPUT_FILENAME);
    QString remote_dataset_parent_dir = _kvl.getValue(DATASET_PARAM_REMOTE_DATASET_PARENT_DIR);

//    m_job_extra_parameters.clear();
    EnrichedFormWidget* param_widget;

    /* update parameter value for dataset path */
    param_widget = m_values_widgets.value(REMOTE_DATASET_STRUCTURE).value(DATASET_PARAM_REMOTE_DATASET_DIR);
    param_widget->setValue(remote_dataset_dir);

    m_job_extra_parameters.insert(DATASET_PARAM_REMOTE_DATASET_DIR);

    /* update parameter value for navigation file */
    param_widget = m_values_widgets.value(REMOTE_DATASET_STRUCTURE).value(DATASET_PARAM_REMOTE_NAVIGATION_FILE);
    param_widget->setValue(remote_navigation_file);

    m_job_extra_parameters.insert(DATASET_PARAM_REMOTE_NAVIGATION_FILE);

    /* update parameter value for output dir */
    param_widget = m_values_widgets.value(REMOTE_DATASET_STRUCTURE).value(DATASET_PARAM_REMOTE_OUTPUT_DIR);
    param_widget->setValue(remote_result_path);

    m_job_extra_parameters.insert(DATASET_PARAM_REMOTE_OUTPUT_DIR);

    /* update parameter value for output file name */
    param_widget = m_values_widgets.value(REMOTE_DATASET_STRUCTURE).value(DATASET_PARAM_REMOTE_OUTPUT_FILENAME);
    param_widget->setValue(remote_output_file);

    m_job_extra_parameters.insert(DATASET_PARAM_REMOTE_OUTPUT_FILENAME);

    /* update parameter value for dataset parent dir */
    param_widget = m_values_widgets.value(REMOTE_DATASET_STRUCTURE).value(DATASET_PARAM_REMOTE_DATASET_PARENT_DIR);
    param_widget->setValue(remote_dataset_parent_dir);

    m_job_extra_parameters.insert(DATASET_PARAM_REMOTE_DATASET_PARENT_DIR);
}

void MatisseParametersManager::setIconFactory(MatisseIconFactory *_icon_factory)
{
    m_icon_factory = _icon_factory;
}

bool MatisseParametersManager::saveParametersValues(QString _entity_name, bool _is_assembly_template)
{
    QString path;

    if (_is_assembly_template) {
        path = "xml/assemblies/parameters/";
        m_job_extra_parameters.clear();
    } else {
        path = "xml/jobs/parameters/";
    }

    QString filename = path.append(_entity_name).replace(' ', '_').append(".xml");
    bool save_status = writeParametersFile(filename, true);
    return save_status;
}

bool MatisseParametersManager::loadParameters(QString _entity_name, bool _is_assembly_template)
{
    QString path;

    if (_is_assembly_template) {
        path = "xml/assemblies/parameters/";
        m_selected_assembly = _entity_name;
    } else {
        path = "xml/jobs/parameters/";
    }

    QString filename = path.append(_entity_name).replace(' ','_').append(".xml");
    bool read_status = readParametersFile(filename, _is_assembly_template);
    m_full_parameters_widget->clearModifications();
    return read_status;
}

void MatisseParametersManager::restoreParametersDefaultValues()
{
    qDebug() << "Restoring parameters values to default";

    foreach (QString struct_name, m_values_widgets.keys()) {
        QMap<QString, EnrichedFormWidget*> struct_params = m_values_widgets.value(struct_name);
        foreach (EnrichedFormWidget* widget, struct_params.values()) {
            widget->restoreDefaultValue();
        }
    }
}

void MatisseParametersManager::createJobParametersFile(QString _assembly_name, QString _job_name, KeyValueList _kvl)
{
    qDebug() << QString("Creating job parameters file for job '%1'").arg(_job_name);

    if (_assembly_name != m_selected_assembly) {
        qCritical() << QString("Job owner assembly '%1 does not match currently selected assembly '%2'")
                       .arg(_assembly_name).arg(m_selected_assembly);
        return;
    }

    pushDatasetParameters(_kvl);

    QString job_parameters_filename = QString("xml/jobs/parameters/").append(_job_name).append(".xml");
    writeParametersFile(job_parameters_filename);
}

bool MatisseParametersManager::writeParametersFile(QString _parameters_filename, bool _overwrite)
{

    /* creation de l'arborescence si nécessaire */
    QDir parent_folder =  QFileInfo(_parameters_filename).dir();
    if (!parent_folder.exists()) {
        parent_folder.mkpath(".");
    }

    QFile parameters_file(_parameters_filename);

    if (!_overwrite && parameters_file.exists()) {
        qCritical() << QString("Job parameters file '%1' already exists").arg(_parameters_filename);
        return false;
    }

    if (!parameters_file.open(QIODevice::WriteOnly)) {
        qCritical() << QString("Could not open job parameters file '%1' for writing").arg(_parameters_filename);
        return false;
    }

    QXmlStreamWriter writer(&parameters_file);
    writer.setCodec("UTF-8");
    writer.setAutoFormatting(true);
    writer.writeStartDocument();
    writer.writeStartElement("JobParameters");
    writer.writeNamespace("http://www.w3.org/2001/XMLSchema-instance", "xsi");
    writer.writeAttribute("xsi:noNamespaceSchemaLocation", "../../../schemas/JobParameters.xsd");

    foreach(QString struct_name, m_structures.keys()) {
        writer.writeStartElement("Structure");
        writer.writeAttribute("name", struct_name);
        Structure structure = m_structures[struct_name];
        for (int no_group = 0; no_group < structure.groups_names.length(); no_group++) {
            QString group_name = structure.groups_names.at(no_group);
            if ( group_name != "") {
                writer.writeComment(group_name);
            }

            foreach(Parameter parameter, structure.parameters_groups.at(no_group).parameters) {
                QString parameter_name = parameter.name;

                if (!m_expected_parameters.contains(parameter_name) && (!m_job_extra_parameters.contains(parameter_name))) {
                    qDebug() << QString("Parameter '%1' not expected, skipping...").arg(parameter_name);
                    continue;
                }

                QString value = getValue(struct_name, parameter_name);
                writer.writeStartElement("Parameter");
                writer.writeAttribute("name", parameter_name);
                writer.writeCharacters(value);
                writer.writeEndElement();
            }
        }

        writer.writeEndElement();
    }

    writer.writeEndDocument();

    parameters_file.flush();
    parameters_file.close();

    return true;
}



bool MatisseParametersManager::readParametersFile(QString _filename, bool _is_assembly_template)
{
    QString file_type = (_is_assembly_template) ? "assembly template" : "job";
    qDebug() << QString("Reading %1 parameters file %2").arg(file_type).arg(_filename);

    QFile parameters_file(_filename);
    if (!parameters_file.exists()) {
        qCritical() << QString("Parameters file '%1' not found").arg(_filename);
        return false;
    }

    if (!parameters_file.open(QIODevice::ReadOnly)) {
        qCritical() << QString("Error opening parameters file '%1'").arg(_filename);
        return false;
    }

    if (!_is_assembly_template) {
        m_job_extra_parameters.clear();
    }

    QXmlStreamReader reader(&parameters_file);

    QString current_structure;
    bool parsing_ok = true;

    while(!reader.atEnd()) {
        /* Read next element.*/
        QXmlStreamReader::TokenType token = reader.readNext();
        /* If token is just StartDocument, we'll go to next.*/
        if(token == QXmlStreamReader::StartDocument) {
            continue;
        }
        /* If token is StartElement, we'll see if we can read it.*/
        if(token == QXmlStreamReader::StartElement) {
            QString tag_name = reader.name().toString();

            if (tag_name == "Structure") {
                QXmlStreamAttributes attributes = reader.attributes();
                current_structure = attributes.value("name").toString();

                if (!m_structures.contains(current_structure)) {
                    qCritical() << QString("Structure '%1' from parameters file not found in parameters dictionnary").arg(current_structure);
                    current_structure = "";
                    continue;
                }

            } else if (tag_name == "Parameter") {
                QXmlStreamAttributes attributes = reader.attributes();
                QString param_name = attributes.value("name").toString();

                if (current_structure.isEmpty()) {
                    qWarning() << QString("Parameter '%1' associated to unknown structure in parameters file, skipping...").arg(param_name);
                    continue;
                }

                if (!m_structure_by_parameter.contains(param_name)) {
                    qCritical() << QString("Parameter '%1' from parameters file not found in dictionnary").arg(param_name);
                    continue;
                }

                QString dico_structure_name = m_structure_by_parameter.value(param_name);

                if (current_structure != dico_structure_name) {
                    qWarning() << QString("Parameter '%1' associated to '%2' structure in parameters file instead of '%3' in dictionnary")
                                  .arg(param_name).arg(current_structure).arg(dico_structure_name);
                }

                bool is_extra_dataset_param = false;
                bool is_remote_dataset_param = false;

                if (!m_expected_parameters.contains(param_name)) {
                    if (!_is_assembly_template && m_dataset_param_names.contains(param_name)) {
                        qDebug() << QString("Extra parameter '%1' found in job parameters file. Keeping as dataset parameter").arg(param_name);
                        is_extra_dataset_param = true;
                    } else if (!_is_assembly_template && m_remote_dataset_param_names.contains(param_name)) {
                       qDebug() << QString("Extra parameter '%1' found in job parameters file. Keeping as remote dataset parameter").arg(param_name);
                       is_extra_dataset_param = true;
                       is_remote_dataset_param = true;
                    }
                    else {
                        // Signaler incoherence et ignorer parametre
                        qWarning() << QString("Parameter '%1' found in parameters file is not referenced as expected by the assembly, skipping...")
                                      .arg(param_name);
                        continue;
                    }
                }

                /* Dataset parameters are excluded from assembly parameters (generic template) */
                /* Since dataset parameters are not to be expected parameters, we should never reach this */
                if (_is_assembly_template && (m_dataset_param_names.contains(param_name) || m_remote_dataset_param_names.contains(param_name))) {
                    qWarning() << QString("Dataset parameter '%1' was referenced as expected").arg(param_name);
                    continue;
                }

                QWidget* param_widget = m_values_widgets.value(dico_structure_name).value(param_name);

                if (!param_widget) {
                    qCritical() << QString("Widget for parameter '%1' is null").arg(param_name);
                    continue;
                }

                QString value = reader.readElementText();

                EnrichedFormWidget* actual_param_widget = static_cast<EnrichedFormWidget *>(param_widget);
                actual_param_widget->setValue(value);

                if (is_extra_dataset_param) {
                    m_job_extra_parameters.insert(param_name);

                    if (!is_remote_dataset_param) { // remote dataset parameters are never shown
                        actual_param_widget->show(); // show extra parameter not defined in assembly

                        /* show dataset param group */
                        QMap<QString, QWidget*> structure_groups_widgets = m_groups_widgets.value(dico_structure_name);
                        QWidget* group_widget = structure_groups_widgets.value(param_name);
                        group_widget->show();
                    }
                }
            }

        }

        if (reader.hasError()) {
            qCritical() << "Error parsing parameters file:\n" << reader.errorString();
            //            parsingOk = false;
            //            break;
        }
    }

    reader.clear();
    parameters_file.close();

    return parsing_ok;
}


bool MatisseParametersManager::getRange(Parameter _param, QVariant &_min_value, QVariant &_max_value)
{
    QString range = _param.range;
    if (range.startsWith("{$")) {
        // recherche enums defini a l'exterieur
        // retourne premiere et derniere valeur
    } else if (m_set_range_expr.exactMatch(range)) {
        // recherche des enums definis a l'interieur // String obligatoirement ou int implicite
        // retourne premiere et derniere valeur
    } else if (m_interval_range_expr.exactMatch(range)) {
        // recherche min max
        _min_value = range.mid(1, range.size()-2).split(",").at(0);
        _max_value = range.mid(1, range.size()-2).split(",").at(1);
        if (range.startsWith("]")) {
            if (_param.type == PAR_INT) {
                if (_min_value.toString() != "-inf") {
                    _min_value = _min_value.toInt() + 1;
                }
            } else if ((_param.type == PAR_FLOAT) || (_param.type == PAR_DOUBLE)) {
                if (_min_value.toString() != "-inf") {
                    _min_value = _min_value.toDouble() + m_epsilon;
                }
            }
        }
        if (range.endsWith("[")) {
            if (_param.type == PAR_INT) {
                if (_max_value.toString() != "inf") {
                    _max_value = _max_value.toInt() - 1;
                }
            } else if ((_param.type == PAR_FLOAT) || (_param.type == PAR_DOUBLE)) {
                if (!_max_value.toString().endsWith("inf")) {
                    _max_value = _max_value.toDouble() - m_epsilon;
                }
            }

        }
    }

    return true;
}

QString MatisseParametersManager::getValue(QString _struct_name, QString _parameter_name)
{
    QWidget * widget = m_values_widgets.value(_struct_name).value(_parameter_name);
    QString value;

    if (widget == NULL) {
        qDebug() << "Pas de widget pour" << _struct_name << "->" << _parameter_name;
        // on est en user, on prend la valeur par defaut
        Structure current_struct =  m_structures.value(_struct_name);
        foreach(ParametersGroup parameters_group, current_struct.parameters_groups) {
            qint32 indexParam = parameters_group.parameters_names.indexOf(_parameter_name);
            if (indexParam > -1) {
                return parameters_group.parameters[indexParam].value.toString();
            }
        }
        return value;
    }
    else {
        value = (qobject_cast<EnrichedFormWidget *>(widget))->currentValue();
    }


    return value;
}


qint32 MatisseParametersManager::getIntValue(QVariant _value)
{
    qint32 ret_value;

    if (_value.toString() == "-inf") {
        ret_value = -InfInt;
    } else if (_value.toString() == "inf") {
        ret_value = InfInt;
    } else {
        ret_value = _value.toInt();
    }

    return ret_value;
}

bool MatisseParametersManager::getBoolValue(QVariant _value)
{
    bool ret_value;

    if (_value.toString() == "true") {
        ret_value = true;
    } else if (_value.toString() == "false") {
        ret_value = false;
    } else {
        ret_value = _value.toBool();
    }

    return ret_value;
}

double MatisseParametersManager::getDoubleValue(QVariant _value)
{
    double ret_value;

    if (_value.toString() == "-inf") {
        ret_value = -InfDouble;
    } else if (_value.toString() == "inf") {
        ret_value = InfDouble;
    } else {
        ret_value = _value.toDouble();
    }

    return ret_value;
}

QStringList MatisseParametersManager::getNumList(Parameter _param)
{
    QString values_str = _param.value.toString();

    values_str = values_str.simplified().replace(" ", "");
    //    valuesStr = valuesStr.mid(1, valuesStr.length()-2);

    values_str.replace("inf",m_inf_str);

    quint16 nb_vars = _param.parameter_size.width() * _param.parameter_size.height();

    QStringList ret_value;

    if (m_matrix_values_expr.exactMatch(values_str)) {
        ret_value = values_str.split(";");
    }

    for (int no_var = ret_value.length(); no_var < nb_vars; no_var++) {
        ret_value.append("0");
    }

    return ret_value;
}

QStringList MatisseParametersManager::getEnums(Parameter _param)
{
    QStringList ret;
    QString range = _param.range;
    if (range.startsWith("{$")) {
        range = range.mid(2, range.size()-3).toUpper();
        if (m_enums.contains(range)) {
            foreach(eEnumValue value, m_enums[range].values) {
                ret.append(value.text);
            }
        }
    } else {
        range  = range.mid(1, range.size()-2);
        ret << range.split(",");
    }

    return ret;
}

} // namespace matisse


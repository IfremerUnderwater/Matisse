#ifndef MATISSE_MATISSE_PARAMETERS_MANAGER_H_
#define MATISSE_MATISSE_PARAMETERS_MANAGER_H_

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
#include "parameters_common.h"
#include "enriched_form_widget.h"
#include "parameters_widget_skeleton.h"
#include "matisse_parameters.h"
#include "key_value_list.h"
#include "enriched_combo_box.h"
#include "enriched_cam_combo_box.h"
#include "enriched_line_edit.h"
#include "enriched_list_box.h"
#include "enriched_spin_box.h"
#include "enriched_double_spin_box.h"
#include "enriched_check_box.h"
#include "enriched_table_widget.h"
#include "enriched_file_chooser.h"
#include "MatisseDictionnaryLabels.h"
#include "parameters_header_button.h"
#include "matisse_icon_factory.h"

namespace matisse {

class MatisseParametersManager : public QObject
{
    Q_OBJECT
public:
    explicit MatisseParametersManager(QObject *_parent = 0);

    bool readDictionnaryFile(QString _xml_filename);
    bool addUserModuleForParameter(QString _user_module, QString _structure_name, QString _param_name);
    bool removeUserModuleForParameter(QString _user_module, QString _structure_name, QString _param_name);
    bool clearExpectedParameters();

    bool addParameter(QString _struct_name, QString _group_name, QString _group_text, QXmlStreamAttributes _attributes);
    bool addEnum(QString _enums_name, QXmlStreamAttributes _attributes);
    ParametersWidgetSkeleton *generateParametersWidget(QWidget *_owner);
    ParametersWidgetSkeleton * parametersWidget() { return m_full_parameters_widget; }

    bool saveParametersValues(QString _entity_name, bool _is_assembly_template);
    bool loadParameters(QString _entity_name, bool _is_assembly_template);
    void restoreParametersDefaultValues();

    void createJobParametersFile(QString _assembly_name, QString _job_name, KeyValueList _kvl);

    void applyApplicationContext(bool _is_expert, bool _is_programming);
    void toggleReadOnlyMode(bool _is_read_only);
    void pullDatasetParameters(KeyValueList &_kvl);
    void pullRemoteDatasetParameters(KeyValueList &_kvl);
    void pushDatasetParameters(KeyValueList _kvl);
    void pushRemoteDatasetParameters(KeyValueList _kvl);
    void pushPreferredDatasetParameters(KeyValueList _kvl);

    QString getParameterValue(QString _parameter_name);

    void setIconFactory(MatisseIconFactory *_icon_factory);

signals:

protected slots:
    void sl_translateParameters();
    void sl_foldUnfoldLevelParameters();

private:
    void loadStaticCollections();
    bool getRange(Parameter _param, QVariant &_min_value, QVariant &_max_value);
    QString getValue(QString _struct_name, QString _parameter_name);
    qint32 getIntValue(QVariant _value);
    bool getBoolValue(QVariant _value);
    double getDoubleValue(QVariant _value);
    QStringList getNumList(Parameter _param);
    QStringList getEnums(Parameter _param);
    bool readParametersFile(QString _filename, bool _is_assembly_template);
    bool writeParametersFile(QString _parameters_filename, bool _overwrite=false);
    void generateLevelParametersWidget(eParameterLevel _level);
    void translateHeaderButtons();
    void retranslateLevelGroups(eParameterLevel _level);
    void checkDictionnaryComplete();

    MatisseDictionnaryLabels m_dictionnary_labels;

    QMap<QString, Structure> m_structures;
    QMap<QString, ParametersGroup> m_groups;
    QMap<QString, Parameter> m_parameters;
    QStringList m_structures_names;
    QMap<QString, Enums> m_enums;
    QMap<QString, QString> m_structure_by_parameter;
    QMap<QString, QSet<QString>*> m_expected_parameters;
    QMap<QString, QString> m_group_by_parameter;
    QMap<eParameterLevel, QList<QString>*> m_parameters_by_level;
    QMap<QString, QSet<QString>*> m_expected_groups;
    QSet<QString> m_job_extra_parameters;
    QMap<QString, QString> m_preferred_dataset_parameters;
    QDateTime m_dico_publication_timestamp;
    QString m_selected_assembly;

    ParametersWidgetSkeleton * m_full_parameters_widget;
    QMap<QString, QMap<QString, QWidget*> > m_groups_widgets;
    QMap<QString, QMap<QString, EnrichedFormWidget*> > m_values_widgets;
    QMap<QString, ParametersWidgetSkeleton*> m_dialogs;

    QMap<QString, EnrichedFormWidget*> m_value_widgets_by_param_name;
    QMap<eParameterLevel, QMap<QString, QGroupBox*>* > m_group_widgets_by_level;

    QMap<eParameterLevel, ParametersHeaderButton*> m_header_buttons_by_level;
    QMap<eParameterLevel, QWidget*> m_param_containers_by_level;

    bool m_is_read_only_mode;

    MatisseIconFactory *m_icon_factory;

    static QMap<QString, eParameterType> m_enum_types;
    static QMap<QString, eParameterLevel> m_enum_levels;
    static QMap<QString, eParameterShow> m_enum_shows;

    static QSet<QString> m_dataset_param_names;
    static QSet<QString> m_remote_dataset_param_names;

    static QRegExp m_interval_range_expr;
    static QRegExp m_set_range_expr;
    static QRegExp m_matrix_expr;
    static QRegExp m_matrix_values_expr;

    static QString m_inf_str;
    static double m_epsilon;
};

} // namespace matisse

#endif // MATISSE_MATISSE_PARAMETERS_MANAGER_H_

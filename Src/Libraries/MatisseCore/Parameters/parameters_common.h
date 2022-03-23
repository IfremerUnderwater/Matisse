#ifndef MATISSE_PARAMETERS_COMMON_H_
#define MATISSE_PARAMETERS_COMMON_H_

#include <QVariant>

namespace matisse {


#define InfInt qPow(2,31)-1
#define InfDouble exp((double)100)

#define PRECISION_MAX       5;
#define PRECISION_DEFAULT   2;

#define DATASET_STRUCTURE               "dataset_param"
#define DATASET_PARAM_OUTPUT_DIR        "output_dir"
#define DATASET_PARAM_OUTPUT_FILENAME   "output_filename"
#define DATASET_PARAM_DATASET_DIR       "dataset_dir"
#define DATASET_PARAM_NAVIGATION_FILE   "navFile"
#define DATASET_PARAM_NAVIGATION_SOURCE "navSource"

struct eEnumValue {
    QString name;
    QString text;
};

enum eParameterLevel {
    USER,
    ADVANCED,
    EXPERT
};

enum eParameterType {
    UNKNOWN_TYPE,
    PAR_INT,
    PAR_FLOAT,
    PAR_DOUBLE,
    PAR_STRING,
    PAR_BOOL,
    PAR_ENUM,
    PAR_CAMERA
};

enum eParameterShow {
    UNKNOWN_SHOW,
    LINE_EDIT,
    COMBO_BOX,
    LIST_BOX,
    SPIN_BOX,
    DOUBLE_SPIN_BOX,
    CHECK_BOX,
    TABLE,
    SLIDER,
    FILE_SELECTOR_RELATIVE,
    FILE_SELECTOR_ABSOLUTE,
    DIR_SELECTOR_RELATIVE,
    DIR_SELECTOR_ABSOLUTE,
    CAM_COMBO_BOX
};

struct Parameter {
    QString name;
    QString text;
    eParameterLevel level;
    eParameterType type;
    QString type_name;
    QString suffix;
    QSize parameter_size;
    quint8 precision;
    QString format_template;
    eParameterShow show;
    QVariant value;
    QString range;
    bool required;
};

struct Condition {
    QString group_name1;
    QString operator_name1;
    QString group_name2;
    QString operator_name2;
    QString relation;
};

struct ParametersGroup {
    QString name;
    QString text;
    QStringList parameters_names;
    bool has_user_values;
    QList<Parameter> parameters;
};

struct Structure {
    QString name;
    QStringList groups_names;
    QList<ParametersGroup> parameters_groups;
    QList<Condition> conditions;
    bool has_user_values;
};

struct Enums {
    QString name;
    QList<eEnumValue> values;
};

} // namespace matisse

#endif // MATISSE_PARAMETERS_COMMON_H_

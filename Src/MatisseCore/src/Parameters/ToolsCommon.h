#ifndef TOOLSCOMMON_H
#define TOOLSCOMMON_H

#include <QVariant>

namespace MatisseTools {


#define InfInt qPow(2,31)-1
#define InfDouble exp((double)100)

#define PRECISION_MAX       5;
#define PRECISION_DEFAULT   2;

#define DATASET_STRUCTURE               "dataset_param"
#define DATASET_PARAM_OUTPUT_DIR        "output_dir"
#define DATASET_PARAM_OUTPUT_FILENAME   "output_filename"
#define DATASET_PARAM_DATASET_DIR       "dataset_dir"
#define DATASET_PARAM_NAVIGATION_FILE   "navFile"

struct EnumValue {
    QString _name;
    QString _text;
};

enum ParameterLevel {
    USER,
    ADVANCED,
    EXPERT
};

enum ParameterType {
    UNKNOWN_TYPE,
    PAR_INT,
    PAR_FLOAT,
    PAR_DOUBLE,
    PAR_STRING,
    PAR_BOOL,
    PAR_ENUM,
    PAR_CAMERA
};

enum ParameterShow {
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
    QString _name;
    QString _text;
    ParameterLevel _level;
    ParameterType _type;
    QString _typeName;
    QString _suffix;
    QSize _parameterSize;
    quint8 _precision;
    QString _formatTemplate;
    ParameterShow _show;
    QVariant _value;
    QString _range;
    bool _required;
};

struct Condition {
    QString _groupName1;
    QString _operatorName1;
    QString _groupName2;
    QString _operatorName2;
    QString _relation;
};

struct ParametersGroup {
    QString _name;
    QString _text;
    QStringList _parametersNames;
    bool _hasUserValues;
    QList<Parameter> _parameters;
};

struct Structure {
    QString _name;
    QStringList _groupsNames;
    QList<ParametersGroup> _parametersGroups;
    QList<Condition> _conditions;
    bool _hasUserValues;
};

struct Enums {
    QString _name;
    QList<EnumValue> _values;
};
}
#endif // TOOLSCOMMON_H

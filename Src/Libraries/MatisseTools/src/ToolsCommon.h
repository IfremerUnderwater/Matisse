#ifndef TOOLSCOMMON_H
#define TOOLSCOMMON_H

namespace MatisseTools {

#define InfInt qPow(2,31)-1
#define InfDouble exp((double)100)

struct EnumValue {
    QString _name;
    QString _text;
};

enum ParameterType {
    UNKNOWN_TYPE,
    PAR_INT,
    PAR_FLOAT,
    PAR_DOUBLE,
    PAR_STRING,
    PAR_BOOL,
    PAR_ENUM
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
    DIR_SELECTOR_ABSOLUTE
};

struct Parameter {
    QString _name;
    QString _text;
    bool _userModify;
    QString _suffix;
    ParameterType _type;
    QSize _parameterSize;
    QString _typeName;
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

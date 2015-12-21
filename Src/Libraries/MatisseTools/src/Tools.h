#ifndef Tools_H
#define Tools_H

#include <QDialog>
#include <QXmlStreamReader>
#include <QtDebug>
#include <QFile>
#include <QMap>
#include <QHash>
#include <QStringRef>
#include <QScriptEngine>
#include <QScriptValue>
#include <QSize>
#include <QRegExp>
#include <QStringList>
#include "ToolsCommon.h"
#include "EnrichedFormWidget.h"
#include "ParametersWidgetSkeleton.h"
#include "MatisseParameters.h"
#include "KeyValueList.h"
#include "EnrichedComboBox.h"
#include "EnrichedLineEdit.h"
#include "EnrichedListBox.h"
#include "EnrichedSpinBox.h"
#include "EnrichedDoubleSpinBox.h"
#include "EnrichedCheckBox.h"
#include "EnrichedTableWidget.h"
#include "EnrichedFileChooser.h"
/*
namespace Ui {
class Wrapper;
}
*/
namespace MatisseTools {

class Tools : public QObject
{
   Q_OBJECT
    
public:
    explicit Tools(/*QWidget *parent = 0*/);
    ~Tools();
//    bool generateDialogClass(QString xmlFilename, QString processorId, QString outputFilename);

    bool readParametersModelFile(QString xmlFilename, bool append = false);
    bool readUserParametersFile(QString xmlFilename, QString xmlModelFilename = "");

    void eraseDialog();
    ParametersWidgetSkeleton *createDialog(QString structName = "", bool user = false);
    void generateParametersFile(QString filename, QString modelVersion, KeyValueList comments = KeyValueList());
    QStringList structureNames();

    ParametersWidgetSkeleton * createFullParametersDialog(bool user = false);

    QString getModelVersion();

protected slots:
//    void slot_openFileDialog();
    void slot_saveProcess();
/*
    void slot_accept();
    void slot_cancel();
*/
private:
    bool addParameter(QString structName, QString groupName, QXmlStreamAttributes attributes);
    bool addEnum(QString enumsName, QXmlStreamAttributes attributes);
    static void fillStaticValues(bool refill = false);

//    Ui::Wrapper *ui;
    QStringList _fileInfo;
    QMap<QString, Structure> _structures;
    QStringList _structuresNames;
    QMap<QString, Enums> _enums;
    QHash<QString, ParametersWidgetSkeleton*> _dialogs;
//    QWidget * _currentDialog;

    static QMap<QString, ParameterType> _enumTypes;
    static QMap<QString, ParameterShow> _enumShows;
    QVariant minValue(Parameter param);
    QVariant maxValue(Parameter param);
    bool getRange(Parameter param, QVariant &minValue, QVariant &maxValue);
    QStringList getEnums(Parameter param);
    QString getValue(QString structName, QString parameterName);
    void fillValuesFromDialog(QString structName);

    static QRegExp _intervalRangeExpr;
    static QRegExp _setRangeExpr;
    static QRegExp _matrixExpr;
    static QRegExp _matrixValuesExpr;

    static QString _infStr;
    static double _epsilon;

    QString _version;
//    QMap<QString, QMap<QString, QWidget*> > _widgetsMap;
//    QMap<QString, ParameterShow> _valuesShowTypes;
    QHash<QString, QHash<QString, QWidget*> > _valuesWidgets;

    qint32 getIntValue(QVariant value);
    bool getBoolValue(QVariant value);
    qreal getDoubleValue(QVariant value);
    QStringList getNumList(Parameter param);


};
}
#endif // Tools_H

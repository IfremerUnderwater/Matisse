#ifndef MATISSEICONFACTORY_H
#define MATISSEICONFACTORY_H

#include <QMap>
#include <QList>
#include <QIcon>
#include <QFile>
#include <QUrl>

#include "FileUtils.h"
#include "IconizedWidgetWrapper.h"

///
/// Class for changing icon colors
///


namespace MatisseTools {
class MatisseIconSpec {
public:
    QString _sourceIconPath;
    bool _isAppModeDependent;
    bool _isStatusDependent;
    QString _contextColorAlias;
};

class MatisseIconFactory : public QObject
{
    Q_OBJECT

public:
    MatisseIconFactory(QMap<QString, QString> defaultColorPalette, QString defaultStatusColorAlias, QString defaultModeColorAlias1, QString defaultModeColorAlias2);
    ~MatisseIconFactory();

    bool attachIcon(IconizedWidgetWrapper *widgetWrapper, QString sourceIconPath, bool isStatusListener = false, bool isAppModeListener = false, QString contextColorAlias="");
    void detachIcon(IconizedWidgetWrapper *widgetWrapper, bool deleteWrapper);
    void clearObsoleteIcons();

protected slots:
    void slot_updateColorPalette(QMap<QString,QString> newColorPalette);
    void slot_updateExecutionStatusColor(QString newStatusColorAlias);
    void slot_updateAppModeColors(QString modeColorAlias1, QString modeColorAlias2);

private:
    bool generateIcon(IconizedWidgetWrapper *widgetWrapper, QString sourceIconPath, bool applyStatusColor, bool applyAppModeColor, QString contextColorAlias="");
    void setWidgetIcon(IconizedWidgetWrapper *widgetWrapper, QString sourceIconPath);
    bool createNewTempDir();
    void releaseWidgetWrapper(IconizedWidgetWrapper *widgetWrapper, bool deleteWrapper);

    QMap<QString,QString> _currentColorPalette;
    QString _currentStatusColorAlias;
    QString _currentModeColorAlias1;
    QString _currentModeColorAlias2;
    QString _currentTempDirPath;

    QMap<IconizedWidgetWrapper *, MatisseIconSpec> _allWidgets;
    QList<IconizedWidgetWrapper *> _statusListeningWidgets;
    QList<IconizedWidgetWrapper *> _appModeListeningWidgets;
    QMap<QString, QString> _tempDirsByContext;

    bool _isAppModeContext;
};
}

#endif // MATISSEICONFACTORY_H

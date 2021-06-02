#include "matisse_icon_factory.h"

using namespace system_tools;
using namespace MatisseTools;

MatisseIconFactory::MatisseIconFactory(QMap<QString, QString> defaultColorPalette, QString defaultStatusColorAlias, QString defaultModeColorAlias1, QString defaultModeColorAlias2) :
    _currentColorPalette(defaultColorPalette),
    _currentStatusColorAlias(defaultStatusColorAlias),
    _currentModeColorAlias1(defaultModeColorAlias1),
    _currentModeColorAlias2(defaultModeColorAlias2),
    _isAppModeContext(true)
{
    bool created = FileUtils::createTempDirectory(_currentTempDirPath);

    if (!created) {
        qFatal("Impossible to create temp directory, cannot generate UI icons...");
    }
}

MatisseIconFactory::~MatisseIconFactory()
{
    _statusListeningWidgets.clear();
    _appModeListeningWidgets.clear();

    /* release memory allocated for widget wrappers */
    foreach (IconizedWidgetWrapper *widgetWrapper, _allWidgets.keys()) {
        _allWidgets.remove(widgetWrapper);
        delete widgetWrapper;
    }
}

void MatisseIconFactory::setWidgetIcon(IconizedWidgetWrapper *widgetWrapper, QString sourceIconPath)
{
    if (widgetWrapper->hasIcon()) {
        QIcon newIcon(sourceIconPath);
        widgetWrapper->setIcon(newIcon);
    } else {
        QPixmap newPixmap(sourceIconPath);
        widgetWrapper->setPixmap(newPixmap);
    }
}

void MatisseIconFactory::detachIcon(IconizedWidgetWrapper *widgetWrapper, bool deleteWrapper)
{
    if (!_allWidgets.contains(widgetWrapper)) {
        qCritical() << QString("No icon attached to the widget, cannot release icon");
        return;
    }

    releaseWidgetWrapper(widgetWrapper, deleteWrapper);
}

void MatisseIconFactory::clearObsoleteIcons()
{
    foreach (IconizedWidgetWrapper *widgetWrapper, _allWidgets.keys()) {
        if (!widgetWrapper->isValid()) {
            qDebug() << "Iconized widget was destroyed, dereferencing...";
            releaseWidgetWrapper(widgetWrapper, true);
        }
    }
}

bool MatisseIconFactory::attachIcon(IconizedWidgetWrapper *widgetWrapper, QString sourceIconPath, bool isStatusListener, bool isAppModeListener, QString contextColorAlias)
{
    if (_allWidgets.contains(widgetWrapper)) {
        qDebug() << QString("Releasing previous icon for widget");
        detachIcon(widgetWrapper, false);
    }

    QFile sourceIconFile(sourceIconPath);
    if (!sourceIconFile.exists()) {
        qCritical() << QString("Source icon file '%1' could not be found, impossible to create icon").arg(sourceIconPath);
        return false;
    }

    if (!sourceIconPath.toLower().endsWith(".svg")) {
        qWarning() << QString("Icon not a SVG file '%1', colors will not be updated dynamically").arg(sourceIconPath);
        setWidgetIcon(widgetWrapper, sourceIconPath);
        return false;
    }

    bool applyStatusColor = isStatusListener;
    bool applyAppModeColor = isAppModeListener;

    /* if widget is both an app mode and execution status listener, it is initialized with current app mode icon */
    if (applyStatusColor && applyAppModeColor) {
        applyStatusColor = false;
    }

    bool generated = generateIcon(widgetWrapper, sourceIconPath, applyStatusColor, applyAppModeColor, contextColorAlias);
    if (!generated) {
        return false;
    }

    MatisseIconSpec spec;
    spec._sourceIconPath = sourceIconPath;
    spec._isAppModeDependent = isAppModeListener;
    spec._isStatusDependent = isStatusListener;
    spec._contextColorAlias = contextColorAlias;

    _allWidgets.insert(widgetWrapper, spec);

    if (isAppModeListener) {
        _appModeListeningWidgets.append(widgetWrapper);
    }

    if (isStatusListener) {
        _statusListeningWidgets.append(widgetWrapper);
    }

    return true;
}

bool MatisseIconFactory::createNewTempDir()
{
    QString previousTempDirPath = _currentTempDirPath;

    QDir currentTempDir(previousTempDirPath);
    QString filter1 = "*.svg";
    QString filter2 = "*.SVG";
    QStringList filters;
    filters << filter1;
    filters << filter2;

    QStringList contents = currentTempDir.entryList(filters, QDir::Files);
    if (contents.isEmpty()) {
        // temp dir is empty, no need to create new one
        qDebug() << "Icons generation temp dir is empty, keeping current temp dir";
        return true;
    }

    qDebug() << "Creating new temp dir for icons generation...";
    bool created = FileUtils::createTempDirectory(_currentTempDirPath);

    if (!created) {
        qCritical("Failed to create temp directory, using previous icons version...");
        _currentTempDirPath = previousTempDirPath;
        return false;
    }

    return true;
}

void MatisseIconFactory::releaseWidgetWrapper(IconizedWidgetWrapper *widgetWrapper, bool deleteWrapper)
{
//    qDebug() << "Releasing iconized widget wrapper...";

    MatisseIconSpec spec = _allWidgets.value(widgetWrapper);
    if (spec._isStatusDependent) {
        _statusListeningWidgets.removeOne(widgetWrapper);
    }

    if (spec._isAppModeDependent) {
        _appModeListeningWidgets.removeOne(widgetWrapper);
    }

    _allWidgets.remove(widgetWrapper);

    if (deleteWrapper) {
        delete widgetWrapper;
    }
}

void MatisseIconFactory::slot_updateColorPalette(QMap<QString, QString> newColorPalette)
{
    qDebug() << "Updating icons color palette...";

    if (!createNewTempDir()) {
        return;
    }

    _currentColorPalette = newColorPalette;

    /* reinit cache for local context icons */
    _tempDirsByContext.clear();

    foreach (IconizedWidgetWrapper *widgetWrapper, _allWidgets.keys()) {
        if (!widgetWrapper->isValid()) {
            qDebug() << "Iconized widget was destroyed, dereferencing...";
            releaseWidgetWrapper(widgetWrapper, true);
            continue;
        }

        MatisseIconSpec spec = _allWidgets.value(widgetWrapper);

        bool applyStatusColor = spec._isStatusDependent;
        bool applyAppModeColor = spec._isAppModeDependent;

        /* if icon is both status and app mode dependent, it is initialized according to context */
        if (applyStatusColor && applyAppModeColor) {
            applyStatusColor = !_isAppModeContext;
            applyAppModeColor = _isAppModeContext;
        }

        generateIcon(widgetWrapper, spec._sourceIconPath, applyStatusColor, applyAppModeColor, spec._contextColorAlias);
    }
}

void MatisseIconFactory::slot_updateExecutionStatusColor(QString newStatusColorAlias)
{
    qDebug() << QString("Updating execution status color '%1' for icons...").arg(newStatusColorAlias);

    if (!createNewTempDir()) {
        return;
    }

    _isAppModeContext = false;
    _currentStatusColorAlias = newStatusColorAlias;

    foreach (IconizedWidgetWrapper *widgetWrapper, _statusListeningWidgets) {
        if (!widgetWrapper->isValid()) {
            qDebug() << "Iconized widget was destroyed, dereferencing...";
            releaseWidgetWrapper(widgetWrapper, true);
            continue;
        }

        MatisseIconSpec spec = _allWidgets.value(widgetWrapper);
        generateIcon(widgetWrapper, spec._sourceIconPath, true, false, spec._contextColorAlias);
    }
}

void MatisseIconFactory::slot_updateAppModeColors(QString modeColorAlias1, QString modeColorAlias2)
{
    qDebug() << QString("Updating app mode colors '%1' & '%2' for icons...").arg(modeColorAlias1).arg(modeColorAlias2);

    if (!createNewTempDir()) {
        return;
    }

    _isAppModeContext = true;
    _currentModeColorAlias1 = modeColorAlias1;
    _currentModeColorAlias2 = modeColorAlias2;

    foreach (IconizedWidgetWrapper *widgetWrapper, _appModeListeningWidgets) {
        if (!widgetWrapper->isValid()) {
            qDebug() << "Iconized widget was destroyed, dereferencing...";
            releaseWidgetWrapper(widgetWrapper, true);
            continue;
        }

        MatisseIconSpec spec = _allWidgets.value(widgetWrapper);
        generateIcon(widgetWrapper, spec._sourceIconPath, false, true, spec._contextColorAlias);
    }

}

bool MatisseIconFactory::generateIcon(IconizedWidgetWrapper *widgetWrapper, QString sourceIconPath, bool applyStatusColor, bool applyAppModeColor, QString contextColorAlias)
{
    if (!widgetWrapper) {
        qCritical() << "Widget wrapper is null, cannot attach icon";
        return false;
    }

    QString currentTempDirPath;

    if (contextColorAlias.isEmpty()) {
        currentTempDirPath = _currentTempDirPath;
    } else {
        /* Using or creating context cache dir */
        if (_tempDirsByContext.contains(contextColorAlias)) {
            currentTempDirPath = _tempDirsByContext.value(contextColorAlias);
        } else {
            if (!createNewTempDir()) {
                qCritical() << QString("Could not create temp dir for new context '%1', icon '%2' will not be generated").arg(contextColorAlias).arg(sourceIconPath);
                return false;
            }
            currentTempDirPath = _currentTempDirPath;
            _tempDirsByContext.insert(contextColorAlias, _currentTempDirPath);
        }
    }

    QFileInfo sourceIconFileInfo(sourceIconPath);

    QString sourceIconFileName = sourceIconFileInfo.fileName();
    QString targetIconFilePath = currentTempDirPath + "/" + sourceIconFileName;

    QFile targetIconFile(targetIconFilePath);
    if (targetIconFile.exists()) {
        qDebug() << QString("Icon '%1' already generated, using cached version").arg(targetIconFilePath);
        setWidgetIcon(widgetWrapper, targetIconFilePath);
        return true;
    }

    QFile sourceIconFile(sourceIconPath);

    if (!sourceIconFile.open(QIODevice::ReadOnly)) {
        qCritical() << QString("I/O error while opening file '%1', could not read file").arg(sourceIconPath);
        return false;
    }

    QTextStream in(&sourceIconFile);
    QString iconDef = in.readAll();
    sourceIconFile.close();

    QMap<QString,QString> _contextProperties;

    if (contextColorAlias.isEmpty()) {
        if (applyAppModeColor) {
            _contextProperties.insert("context.color1.alias", _currentModeColorAlias1);
            _contextProperties.insert("context.color2.alias", _currentModeColorAlias2);
        } else if (applyStatusColor) {
            _contextProperties.insert("context.color1.alias", _currentStatusColorAlias);
            _contextProperties.insert("context.color2.alias", _currentStatusColorAlias);
        }
    } else {
        /* using provided context color alias */
        _contextProperties.insert("context.color1.alias", contextColorAlias);
        _contextProperties.insert("context.color2.alias", contextColorAlias);
    }

    /* Determine context colors */
    QString contextualizedIconDef;

    if (_contextProperties.isEmpty()) {
        contextualizedIconDef = iconDef;
    } else {
        contextualizedIconDef = StringUtils::substitutePlaceHolders(iconDef, _contextProperties, PLACEHOLDER_PATTERN_XML_2);
    }

    /* resolve day / night colors */
    QString resolvedIconDef = StringUtils::substitutePlaceHolders(contextualizedIconDef, _currentColorPalette, PLACEHOLDER_PATTERN_XML_1);

    if (!targetIconFile.open(QIODevice::WriteOnly)) {
        qCritical() << QString("I/O error while opening file '%1', could not write colorized icon").arg(targetIconFilePath);
        return false;
    }

    QTextStream out(&targetIconFile);
    out << resolvedIconDef;
    out.flush();

    targetIconFile.close();

    setWidgetIcon(widgetWrapper, targetIconFilePath);

    return true;
}

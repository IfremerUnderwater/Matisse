#include "matisse_icon_factory.h"

using namespace system_tools;

namespace matisse {

MatisseIconFactory::MatisseIconFactory(QMap<QString, QString> _default_color_palette, QString _default_status_color_alias, QString _default_mode_color_alias1, QString _default_mode_color_alias2) :
    m_current_color_palette(_default_color_palette),
    m_current_status_color_alias(_default_status_color_alias),
    m_current_mode_color_alias1(_default_mode_color_alias1),
    m_current_mode_color_alias2(_default_mode_color_alias2),
    m_is_app_mode_context(true)
{
    bool created = FileUtils::createTempDirectory(m_current_temp_dir_path);

    if (!created) {
        qFatal("Impossible to create temp directory, cannot generate UI icons...");
    }
}

MatisseIconFactory::~MatisseIconFactory()
{
    m_status_listening_widgets.clear();
    m_app_mode_listening_widgets.clear();

    /* release memory allocated for widget wrappers */
    foreach (IconizedWidgetWrapper *widget_wrapper, m_all_widgets.keys()) {
        m_all_widgets.remove(widget_wrapper);
        delete widget_wrapper;
    }
}

void MatisseIconFactory::setWidgetIcon(IconizedWidgetWrapper *_widget_wrapper, QString _source_icon_path)
{
    if (_widget_wrapper->hasIcon()) {
        QIcon new_icon(_source_icon_path);
        _widget_wrapper->setIcon(new_icon);
    } else {
        QPixmap new_pixmap(_source_icon_path);
        _widget_wrapper->setPixmap(new_pixmap);
    }
}

void MatisseIconFactory::detachIcon(IconizedWidgetWrapper *_widget_wrapper, bool _delete_wrapper)
{
    if (!m_all_widgets.contains(_widget_wrapper)) {
        qCritical() << QString("No icon attached to the widget, cannot release icon");
        return;
    }

    releaseWidgetWrapper(_widget_wrapper, _delete_wrapper);
}

void MatisseIconFactory::clearObsoleteIcons()
{
    foreach (IconizedWidgetWrapper *widget_wrapper, m_all_widgets.keys()) {
        if (!widget_wrapper->isValid()) {
            qDebug() << "Iconized widget was destroyed, dereferencing...";
            releaseWidgetWrapper(widget_wrapper, true);
        }
    }
}

bool MatisseIconFactory::attachIcon(IconizedWidgetWrapper *_widget_wrapper, QString _source_icon_path, bool _is_status_listener, bool _is_app_mode_listener, QString _context_color_alias)
{
    if (m_all_widgets.contains(_widget_wrapper)) {
        qDebug() << QString("Releasing previous icon for widget");
        detachIcon(_widget_wrapper, false);
    }

    QFile source_icon_file(_source_icon_path);
    if (!source_icon_file.exists()) {
        qCritical() << QString("Source icon file '%1' could not be found, impossible to create icon").arg(_source_icon_path);
        return false;
    }

    if (!_source_icon_path.toLower().endsWith(".svg")) {
        qWarning() << QString("Icon not a SVG file '%1', colors will not be updated dynamically").arg(_source_icon_path);
        setWidgetIcon(_widget_wrapper, _source_icon_path);
        return false;
    }

    bool apply_status_color = _is_status_listener;
    bool apply_app_mode_color = _is_app_mode_listener;

    /* if widget is both an app mode and execution status listener, it is initialized with current app mode icon */
    if (apply_status_color && apply_app_mode_color) {
        apply_status_color = false;
    }

    bool generated = generateIcon(_widget_wrapper, _source_icon_path, apply_status_color, apply_app_mode_color, _context_color_alias);
    if (!generated) {
        return false;
    }

    MatisseIconSpec spec;
    spec.m_source_icon_path = _source_icon_path;
    spec.m_is_app_mode_dependent = _is_app_mode_listener;
    spec.m_is_status_dependent = _is_status_listener;
    spec.m_context_color_alias = _context_color_alias;

    m_all_widgets.insert(_widget_wrapper, spec);

    if (_is_app_mode_listener) {
        m_app_mode_listening_widgets.append(_widget_wrapper);
    }

    if (_is_status_listener) {
        m_status_listening_widgets.append(_widget_wrapper);
    }

    return true;
}

bool MatisseIconFactory::createNewTempDir()
{
    QString previous_temp_dir_path = m_current_temp_dir_path;

    QDir current_temp_dir(previous_temp_dir_path);
    QString filter1 = "*.svg";
    QString filter2 = "*.SVG";
    QStringList filters;
    filters << filter1;
    filters << filter2;

    QStringList contents = current_temp_dir.entryList(filters, QDir::Files);
    if (contents.isEmpty()) {
        // temp dir is empty, no need to create new one
        qDebug() << "Icons generation temp dir is empty, keeping current temp dir";
        return true;
    }

    qDebug() << "Creating new temp dir for icons generation...";
    bool created = FileUtils::createTempDirectory(m_current_temp_dir_path);

    if (!created) {
        qCritical("Failed to create temp directory, using previous icons version...");
        m_current_temp_dir_path = previous_temp_dir_path;
        return false;
    }

    return true;
}

void MatisseIconFactory::releaseWidgetWrapper(IconizedWidgetWrapper *_widget_wrapper, bool _delete_wrapper)
{
//    qDebug() << "Releasing iconized widget wrapper...";

    MatisseIconSpec spec = m_all_widgets.value(_widget_wrapper);
    if (spec.m_is_status_dependent) {
        m_status_listening_widgets.removeOne(_widget_wrapper);
    }

    if (spec.m_is_app_mode_dependent) {
        m_app_mode_listening_widgets.removeOne(_widget_wrapper);
    }

    m_all_widgets.remove(_widget_wrapper);

    if (_delete_wrapper) {
        delete _widget_wrapper;
    }
}

void MatisseIconFactory::sl_updateColorPalette(QMap<QString, QString> _new_color_palette)
{
    qDebug() << "Updating icons color palette...";

    if (!createNewTempDir()) {
        return;
    }

    m_current_color_palette = _new_color_palette;

    /* reinit cache for local context icons */
    m_temp_dirs_by_context.clear();

    foreach (IconizedWidgetWrapper *widget_wrapper, m_all_widgets.keys()) {
        if (!widget_wrapper->isValid()) {
            qDebug() << "Iconized widget was destroyed, dereferencing...";
            releaseWidgetWrapper(widget_wrapper, true);
            continue;
        }

        MatisseIconSpec spec = m_all_widgets.value(widget_wrapper);

        bool apply_status_color = spec.m_is_status_dependent;
        bool apply_app_mode_color = spec.m_is_app_mode_dependent;

        /* if icon is both status and app mode dependent, it is initialized according to context */
        if (apply_status_color && apply_app_mode_color) {
            apply_status_color = !m_is_app_mode_context;
            apply_app_mode_color = m_is_app_mode_context;
        }

        generateIcon(widget_wrapper, spec.m_source_icon_path, apply_status_color, apply_app_mode_color, spec.m_context_color_alias);
    }
}

void MatisseIconFactory::sl_updateExecutionStatusColor(QString _new_status_color_alias)
{
    qDebug() << QString("Updating execution status color '%1' for icons...").arg(_new_status_color_alias);

    if (!createNewTempDir()) {
        return;
    }

    m_is_app_mode_context = false;
    m_current_status_color_alias = _new_status_color_alias;

    foreach (IconizedWidgetWrapper *widget_wrapper, m_status_listening_widgets) {
        if (!widget_wrapper->isValid()) {
            qDebug() << "Iconized widget was destroyed, dereferencing...";
            releaseWidgetWrapper(widget_wrapper, true);
            continue;
        }

        MatisseIconSpec spec = m_all_widgets.value(widget_wrapper);
        generateIcon(widget_wrapper, spec.m_source_icon_path, true, false, spec.m_context_color_alias);
    }
}

void MatisseIconFactory::sl_updateAppModeColors(QString _mode_color_alias1, QString _mode_color_alias2)
{
    qDebug() << QString("Updating app mode colors '%1' & '%2' for icons...").arg(_mode_color_alias1).arg(_mode_color_alias2);

    if (!createNewTempDir()) {
        return;
    }

    m_is_app_mode_context = true;
    m_current_mode_color_alias1 = _mode_color_alias1;
    m_current_mode_color_alias2 = _mode_color_alias2;

    foreach (IconizedWidgetWrapper *widget_wrapper, m_app_mode_listening_widgets) {
        if (!widget_wrapper->isValid()) {
            qDebug() << "Iconized widget was destroyed, dereferencing...";
            releaseWidgetWrapper(widget_wrapper, true);
            continue;
        }

        MatisseIconSpec spec = m_all_widgets.value(widget_wrapper);
        generateIcon(widget_wrapper, spec.m_source_icon_path, false, true, spec.m_context_color_alias);
    }

}

bool MatisseIconFactory::generateIcon(IconizedWidgetWrapper *_widget_wrapper, QString _source_icon_path, bool _apply_status_color, bool _apply_app_mode_color, QString _context_color_alias)
{
    if (!_widget_wrapper) {
        qCritical() << "Widget wrapper is null, cannot attach icon";
        return false;
    }

    QString current_temp_dir_path;

    if (_context_color_alias.isEmpty()) {
        current_temp_dir_path = m_current_temp_dir_path;
    } else {
        /* Using or creating context cache dir */
        if (m_temp_dirs_by_context.contains(_context_color_alias)) {
            current_temp_dir_path = m_temp_dirs_by_context.value(_context_color_alias);
        } else {
            if (!createNewTempDir()) {
                qCritical() << QString("Could not create temp dir for new context '%1', icon '%2' will not be generated").arg(_context_color_alias).arg(_source_icon_path);
                return false;
            }
            current_temp_dir_path = m_current_temp_dir_path;
            m_temp_dirs_by_context.insert(_context_color_alias, m_current_temp_dir_path);
        }
    }

    QFileInfo source_icon_file_info(_source_icon_path);

    QString source_icon_file_name = source_icon_file_info.fileName();
    QString target_icon_dile_path = current_temp_dir_path + "/" + source_icon_file_name;

    QFile target_icon_file(target_icon_dile_path);
    if (target_icon_file.exists()) {
        qDebug() << QString("Icon '%1' already generated, using cached version").arg(target_icon_dile_path);
        setWidgetIcon(_widget_wrapper, target_icon_dile_path);
        return true;
    }

    QFile source_icon_file(_source_icon_path);

    if (!source_icon_file.open(QIODevice::ReadOnly)) {
        qCritical() << QString("I/O error while opening file '%1', could not read file").arg(_source_icon_path);
        return false;
    }

    QTextStream in(&source_icon_file);
    QString icon_def = in.readAll();
    source_icon_file.close();

    QMap<QString,QString> _contextProperties;

    if (_context_color_alias.isEmpty()) {
        if (_apply_app_mode_color) {
            _contextProperties.insert("context.color1.alias", m_current_mode_color_alias1);
            _contextProperties.insert("context.color2.alias", m_current_mode_color_alias2);
        } else if (_apply_status_color) {
            _contextProperties.insert("context.color1.alias", m_current_status_color_alias);
            _contextProperties.insert("context.color2.alias", m_current_status_color_alias);
        }
    } else {
        /* using provided context color alias */
        _contextProperties.insert("context.color1.alias", _context_color_alias);
        _contextProperties.insert("context.color2.alias", _context_color_alias);
    }

    /* Determine context colors */
    QString contextualized_icon_def;

    if (_contextProperties.isEmpty()) {
        contextualized_icon_def = icon_def;
    } else {
        contextualized_icon_def = StringUtils::substitutePlaceHolders(icon_def, _contextProperties, PLACEHOLDER_PATTERN_XML_2);
    }

    /* resolve day / night colors */
    QString resolved_icon_def = StringUtils::substitutePlaceHolders(contextualized_icon_def, m_current_color_palette, PLACEHOLDER_PATTERN_XML_1);

    if (!target_icon_file.open(QIODevice::WriteOnly)) {
        qCritical() << QString("I/O error while opening file '%1', could not write colorized icon").arg(target_icon_dile_path);
        return false;
    }

    QTextStream out(&target_icon_file);
    out << resolved_icon_def;
    out.flush();

    target_icon_file.close();

    setWidgetIcon(_widget_wrapper, target_icon_dile_path);

    return true;
}

} // namespace matisse

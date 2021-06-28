#ifndef MATISSE_MATISSE_ICON_FACTORY_H_
#define MATISSE_MATISSE_ICON_FACTORY_H_

#include <QMap>
#include <QList>
#include <QIcon>
#include <QFile>
#include <QUrl>

#include "file_utils.h"
#include "iconized_widget_wrapper.h"

///
/// Class for changing icon colors
///


namespace matisse {

class MatisseIconSpec {
public:
    QString m_source_icon_path;
    bool m_is_app_mode_dependent;
    bool m_is_status_dependent;
    QString m_context_color_alias;
};

class MatisseIconFactory : public QObject
{
    Q_OBJECT

public:
    MatisseIconFactory(QMap<QString, QString> _default_color_palette, QString _default_status_color_alias, QString _default_mode_color_alias1, QString _default_mode_color_alias2);
    ~MatisseIconFactory();

    bool attachIcon(IconizedWidgetWrapper *_widget_wrapper, QString _source_icon_path, bool _is_status_listener = false, bool _is_app_mode_listener = false, QString _context_color_alias="");
    void detachIcon(IconizedWidgetWrapper *_widget_wrapper, bool _delete_wrapper);
    void clearObsoleteIcons();

protected slots:
    void sl_updateColorPalette(QMap<QString,QString> _new_color_palette);
    void sl_updateExecutionStatusColor(QString _new_status_color_alias);
    void sl_updateAppModeColors(QString _mode_color_alias1, QString _mode_color_alias2);

private:
    bool generateIcon(IconizedWidgetWrapper *_widget_wrapper, QString _source_icon_path, bool _apply_status_color, bool _apply_app_mode_color, QString _context_color_alias="");
    void setWidgetIcon(IconizedWidgetWrapper *_widget_wrapper, QString _source_icon_path);
    bool createNewTempDir();
    void releaseWidgetWrapper(IconizedWidgetWrapper *_widget_wrapper, bool _delete_wrapper);

    QMap<QString,QString> m_current_color_palette;
    QString m_current_status_color_alias;
    QString m_current_mode_color_alias1;
    QString m_current_mode_color_alias2;
    QString m_current_temp_dir_path;

    QMap<IconizedWidgetWrapper *, MatisseIconSpec> m_all_widgets;
    QList<IconizedWidgetWrapper *> m_status_listening_widgets;
    QList<IconizedWidgetWrapper *> m_app_mode_listening_widgets;
    QMap<QString, QString> m_temp_dirs_by_context;

    bool m_is_app_mode_context;
};

} // namespace matisse

#endif // MATISSE_MATISSE_ICON_FACTORY_H_

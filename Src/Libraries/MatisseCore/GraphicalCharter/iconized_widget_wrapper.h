#ifndef MATISSE_ICONIZED_WIDGET_WRAPPER_H_
#define MATISSE_ICONIZED_WIDGET_WRAPPER_H_

#include <QIcon>
#include <QPixmap>
#include <QWidget>

namespace matisse {

class IconizedWidgetWrapper : public QObject
{
    Q_OBJECT

public:
    IconizedWidgetWrapper(QObject* _widget, bool _has_icon);

    virtual void setIcon(const QIcon &_icon) {Q_UNUSED(_icon)}
    virtual void setPixmap(const QPixmap &_pixmap) {Q_UNUSED(_pixmap)}

    bool hasIcon() const;
    bool isValid() const;

protected slots:
    void sl_invalidate(QObject *_widget);

private:
    bool m_has_icon;
    bool m_is_valid;
    QObject *m_widget;
};

} // namespace matisse

#endif // MATISSE_ICONIZED_WIDGET_WRAPPER_H_

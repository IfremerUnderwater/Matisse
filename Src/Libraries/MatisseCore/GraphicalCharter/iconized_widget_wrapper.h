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
    IconizedWidgetWrapper(QObject* widget, bool _hasIcon);

    virtual void setIcon(const QIcon &icon) {Q_UNUSED(icon)}
    virtual void setPixmap(const QPixmap &pixmap) {Q_UNUSED(pixmap)}

    bool hasIcon() const;
    bool isValid() const;

protected slots:
    void slot_invalidate(QObject *widget);

private:
    bool _hasIcon;
    bool _isValid;
    QObject *_widget;
};

} // namespace matisse

#endif // MATISSE_ICONIZED_WIDGET_WRAPPER_H_

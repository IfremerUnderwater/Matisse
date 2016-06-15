#ifndef ICONIZEDWIDGETWRAPPER_H
#define ICONIZEDWIDGETWRAPPER_H

#include <QIcon>
#include <QPixmap>
#include <QWidget>

namespace MatisseTools {

class IconizedWidgetWrapper : public QObject
{
    Q_OBJECT

public:
    IconizedWidgetWrapper(QObject* widget, bool _hasIcon);

    virtual void setIcon(const QIcon &icon) {}
    virtual void setPixmap(const QPixmap &pixmap) {}

    bool hasIcon() const;
    bool isValid() const;

protected slots:
    void slot_invalidate(QObject *widget);

private:
    bool _hasIcon;
    bool _isValid;
    QObject *_widget;
};
}

#endif // ICONIZEDWIDGETWRAPPER_H

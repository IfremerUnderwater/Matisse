#ifndef MATISSE_OVERLAY_WIDGET_H_
#define MATISSE_OVERLAY_WIDGET_H_

#include <QWidget>
#include "shader_color.h"

namespace matisse {

class OverlayWidget : public QWidget
{
    Q_OBJECT
public:
    explicit OverlayWidget(QWidget *parent = 0);

    void setMinMax(float _min, float _max) { m_zmin = _min; m_zmax = _max; }

    void setColorPalette(ShaderColor::Palette _palette) { m_colorPalette = _palette; }

protected:
    void paintEvent(QPaintEvent *event);

signals:

public slots:

private:
    double m_zmin;
    double m_zmax;

    ShaderColor::Palette m_colorPalette;
};

} // namespace matisse

#endif // MATISSE_OVERLAY_WIDGET_H_

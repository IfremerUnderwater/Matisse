#ifndef MATISSE_PARAMETERS_FOLD_BUTTON_H_
#define MATISSE_PARAMETERS_FOLD_BUTTON_H_

#include <QtDebug>
#include <QPainter>
#include <QPainterPath>

#include "fold_unfold_button.h"
#include "graphical_charter.h"

namespace matisse {

class ParametersFoldButton : public FoldUnfoldButton
{
    Q_OBJECT

public:
    ParametersFoldButton(QWidget *_parent=0);

protected:
    void paintEvent(QPaintEvent *_event);
    void showNewState();

protected slots:
    void sl_updateColorPalette(QMap<QString,QString> _new_color_palette);

private:
    void drawFoldArrow(QPainter &_painter, QColor _paint_color);
    void drawUnfoldArrow(QPainter &_painter, QColor _paint_color);

    QString m_current_paint_color_ref;

    static const quint8 ARROW_LINE_WIDTH;
    static const quint8 ARROW_LINE_HEIGHT;
    static const quint8 ARROW_TIP_WIDTH;
    static const quint8 ARROW_WING_HEIGHT;
};

} // namespace matisse

#endif // MATISSE_PARAMETERS_FOLD_BUTTON_H_

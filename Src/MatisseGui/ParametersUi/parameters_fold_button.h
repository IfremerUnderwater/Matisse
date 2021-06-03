#ifndef MATISSE_PARAMETERS_FOLD_BUTTON_H_
#define MATISSE_PARAMETERS_FOLD_BUTTON_H_

#include <QtDebug>
#include <QPainter>

#include "fold_unfold_button.h"
#include "graphical_charter.h"

namespace matisse {

class ParametersFoldButton : public FoldUnfoldButton
{
    Q_OBJECT

public:
    ParametersFoldButton(QWidget *parent=0);

protected:
    void paintEvent(QPaintEvent *event);
    void showNewState();

protected slots:
    void slot_updateColorPalette(QMap<QString,QString> newColorPalette);

private:
    void drawFoldArrow(QPainter &painter, QColor paintColor);
    void drawUnfoldArrow(QPainter &painter, QColor paintColor);

    QString _currentPaintColorRef;

    static const quint8 ARROW_LINE_WIDTH;
    static const quint8 ARROW_LINE_HEIGHT;
    static const quint8 ARROW_TIP_WIDTH;
    static const quint8 ARROW_WING_HEIGHT;
};

} // namespace matisse

#endif // MATISSE_PARAMETERS_FOLD_BUTTON_H_

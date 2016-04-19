#ifndef PARAMETERSFOLDBUTTON_H
#define PARAMETERSFOLDBUTTON_H

#include <QtDebug>
#include <QPainter>

#include "FoldUnfoldButton.h"
#include "GraphicalCharter.h"

using namespace MatisseTools;

namespace MatisseServer {

class ParametersFoldButton : public FoldUnfoldButton
{
public:
    ParametersFoldButton(QWidget *parent=0);

protected:
    void paintEvent(QPaintEvent *event);
    void showNewState();

private:
    void drawFoldArrow(QPainter &painter, QColor paintColor);
    void drawUnfoldArrow(QPainter &painter, QColor paintColor);

    static const quint8 ARROW_LINE_WIDTH;
    static const quint8 ARROW_LINE_HEIGHT;
    static const quint8 ARROW_TIP_WIDTH;
    static const quint8 ARROW_WING_HEIGHT;
};

}

#endif // PARAMETERSFOLDBUTTON_H

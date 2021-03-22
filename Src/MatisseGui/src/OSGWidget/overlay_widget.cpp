#include "overlay_widget.h"
#include <QPainter>

OverlayWidget::OverlayWidget(QWidget *parent) : QWidget(parent)
{
    resize(80, 255);

    setAutoFillBackground(false);
    setWindowFlags(Qt::FramelessWindowHint);
    //setAttribute(Qt::WA_TranslucentBackground);
    setAttribute(Qt::WA_TransparentForMouseEvents);
}

void OverlayWidget::paintEvent(QPaintEvent * /*event*/)
{
    QPainter painter(this);
    //painter.setRenderHint(QPainter::Antialiasing);

    painter.setOpacity(1.0);

    QPen pen(Qt::lightGray, 1, Qt::SolidLine);
    painter.setPen(pen);
    QFont font = painter.font();
    font.setPixelSize(12);
    font.setBold(true);
    painter.setFont(font);

    QString min = QString::number(m_zmin ,'f',1);
    painter.drawText( 0, height(), min + "m");

    QString max = QString::number(m_zmax ,'f',1);
    painter.drawText( 0, 10, max + "m");

    // draw palette
    for(int i=0; i<256; i++)
    {
        QColor color = ShaderColor::color(i / 255.0, m_colorPalette);
        pen.setColor(color);
        painter.setPen(pen);
        int y = height() - i;
        painter.drawLine( width() - 20,y , width(), y);
    }
}


#ifndef SHADERCOLOR_H
#define SHADERCOLOR_H

#include <QColor>

class ShaderColor
{
public:

    enum Palette
    {
        Rainbow,
        BlueToYellow
    };

    // warniong : _val = z in range 0..1
    static QColor color(double _val, const Palette _palette = Rainbow);
    static const char * shaderSource(const Palette _palette = Rainbow);
    static const char * paletteName(const Palette _palette = Rainbow);

};

#endif // SHADERCOLOR_H

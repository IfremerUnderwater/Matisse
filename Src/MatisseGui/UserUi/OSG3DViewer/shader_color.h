#ifndef SHADERCOLOR_H
#define SHADERCOLOR_H

#include <QColor>

namespace matisse {

class ShaderColor
{
public:

    enum ePalette
    {
        RAINBOW,
        BLUE_TO_YELLOW
    };

    // warniong : _val = z in range 0..1
    static QColor m_color(double _val, const ePalette _palette = RAINBOW);
    static const char * m_shader_source(const ePalette _palette = RAINBOW);
    static const char * m_palette_name(const ePalette _palette = RAINBOW);

};

} // namespace matisse

#endif // SHADERCOLOR_H

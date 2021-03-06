#include "shader_color.h"
#include <osg/Vec3f>

namespace matisse {

static osg::Vec3f HSVSpectrum(float x)
{
    float y = 1.0;
    float z = 1.0;
    osg::Vec3f RGB(x, y, z);
    float hi = floor(x * 6.0);
    float f = x * 6.0 - hi;
    float p = z * (1.0-y);
    float q = z * (1.0-y*f);
    float t = z * (1.0-y*(1.0-f));
    if(y != 0.0)
    {
        if (hi == 0.0 || hi == 6.0) { RGB = osg::Vec3f(z, t, p); }
        else if (hi == 1.0) { RGB = osg::Vec3f(q, z, p); }
        else if (hi == 2.0) { RGB = osg::Vec3f(p, z, t); }
        else if (hi == 3.0) { RGB = osg::Vec3f(p, q, z); }
        else if (hi == 4.0) { RGB = osg::Vec3f(t, p, z); }
        else { RGB = osg::Vec3f(z, p, q); }
    }
    return RGB;

}

// val in 0..1
static QColor RainbowColor(float _val)
{
    float v2 = (-_val * 0.75) + 0.67;
    if(v2 > 1.0)
        v2 = v2- 1.0;
    osg::Vec3f RGB = HSVSpectrum(v2);
    QColor color(RGB[0]*255, RGB[1]*255, RGB[2]*255);

    return color;
}

// val in 0..1
static QColor BlueToYellowColor(float _val)
{
    QColor color(_val*255, _val*255, 128);

    return color;
}

QColor ShaderColor::m_color(double _val, const ShaderColor::ePalette _palette)
{
    float val = _val;
    if(val < 0)
        val = 0;
    if(val > 1.0)
        val = 1.0;

    switch(_palette)
    {
    case RAINBOW:
        return RainbowColor(val);
        break;
    case BLUE_TO_YELLOW:
        return BlueToYellowColor(val);
        break;
    }

    return Qt::white;
}

static const char * RAINBOW_SHADER_SOURCE =
        "vec3 HSVSpectrum(float x)"
        "{"
        " float y = 1.0;"
        " float z = 1.0;"
        " vec3 RGB = vec3(x, y, z);"
        " float hi = floor(x * 6.0);"
        " float f = x * 6.0 - hi;"
        " float p = z * (1.0-y);"
        " float q = z * (1.0-y*f);"
        " float t = z * (1.0-y*(1.0-f));"
        ""
        " if(y != 0.0)"
        " {"
        "   if (hi == 0.0 || hi == 6.0) { RGB = vec3(z, t, p); }"
        "   else if (hi == 1.0) { RGB = vec3(q, z, p); }"
        "   else if (hi == 2.0) { RGB = vec3(p, z, t); }"
        "   else if (hi == 3.0) { RGB = vec3(p, q, z); }"
        "   else if (hi == 4.0) { RGB = vec3(t, p, z); }"
        "   else { RGB = vec3(z, p, q); }"
        " }"
        " return RGB;"
        "}"

        "vec3 colorPalette(float val)"
        "{"
        "    if(val < 0.0)"
        "      val = 0.0;"
        "    if(val > 1.0)"
        "      val = 1.0;"
        "    float v2 = (-val * 0.75) + 0.67;"
        "    if(v2 > 1.0)"
        "      v2 = v2- 1.0;"
        "    vec3 RGB = HSVSpectrum(v2);"
        "    return RGB;"
        "}";

static const char *BLUE_TO_YELLOW_SHADER_SOURCE =
        "vec3 colorPalette(float val)"
        "{"
        "    if(val < 0.0)"
        "      val = 0.0;"
        "    if(val > 1.0)"
        "      val = 1.0;"
        "    vec3 RGB = vec3(val, val, 0.5);"
        "    return RGB;"
        "}";


const char *ShaderColor::m_shader_source(const ShaderColor::ePalette _palette)
{
    switch(_palette)
    {
    case RAINBOW:
        return RAINBOW_SHADER_SOURCE;
        break;

    case BLUE_TO_YELLOW:
        return BLUE_TO_YELLOW_SHADER_SOURCE;
        break;

    default:

        return
                "vec3 colorPalette(float val)"
                "{"
                "    vec3 RGB;"
                "    RGB.r = val;"
                "    RGB.g = val;"
                "    RGB.b = val;"
                "    return RGB;"
                "}";
    }

    return "";
}

const char *ShaderColor::m_palette_name(const ShaderColor::ePalette _palette)
{
    switch(_palette)
    {
    case RAINBOW:
        return "Rainbow";
        break;
    case BLUE_TO_YELLOW:
        return "BlueToYellow";
        break;

    default:
        return "";
    }
}

} // namespace matisse

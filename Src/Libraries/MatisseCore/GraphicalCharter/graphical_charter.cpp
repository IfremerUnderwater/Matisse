#include "graphical_charter.h"
#include <QGuiApplication>
#include <QScreen>

using namespace MatisseCommon;

//// Constructor & Dectructor
GraphicalCharter &GraphicalCharter::instance()
{
    // create instance by lazy initialization
    // guaranteed to be destroyed
    static GraphicalCharter instance;
    return instance;
}

GraphicalCharter::GraphicalCharter() {
    m_dpi = QGuiApplication::primaryScreen()->logicalDotsPerInch();
}

GraphicalCharter::~GraphicalCharter() {

}

double GraphicalCharter::dpi() const
{
    return m_dpi;
}

int GraphicalCharter::dpiScaled(int _ref_pixel_size)
{
    return (int)((double)_ref_pixel_size*(m_dpi)/((double)REF_DPI));
}


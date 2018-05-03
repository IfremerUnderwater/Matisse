#include "GraphicalCharter.h"
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
    m_dpi = QGuiApplication::primaryScreen()->physicalDotsPerInch();
}

GraphicalCharter::~GraphicalCharter() {

}

double GraphicalCharter::dpi() const
{
    return m_dpi;
}


#include <QtGui/QApplication>
#include "TestAppliQt.h"

#include "qgsapplication.h"
#include "qgsproviderregistry.h"
#include "symbology-ng/qgssinglesymbolrendererv2.h"
#include "qgsmaplayerregistry.h"
#include "qgsvectorlayer.h"
#include "qgsmapcanvas.h"

int main(int argc, char *argv[])
{
    QApplication a(argc, argv);
    TestAppliQt w;
    w.show();
    
    return a.exec();
}

#include <QtGui/QApplication>
#include "GraphicalMosaicBuilder.h"

int main(int argc, char *argv[])
{
    QApplication a(argc, argv);
    GraphicalMosaicBuilder w;
    w.show();
    
    return a.exec();
}

#include <QtGui/QApplication>
#include "ProcessorExtractor.h"

int main(int argc, char *argv[])
{
    QApplication a(argc, argv);
    ProcessorExtractor w;
    w.show();
    
    return a.exec();
}

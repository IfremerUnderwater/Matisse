#include <QtGui/QApplication>
#include "ClientTest.h"

int main(int argc, char *argv[])
{
    QApplication a(argc, argv);
    ClientTest w;
    w.show();
    
    return a.exec();
}

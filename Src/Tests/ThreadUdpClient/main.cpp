#include "threadudpclientgui.h"
#include <QApplication>

int main(int argc, char *argv[])
{
    QApplication a(argc, argv);
    ThreadUdpClientGUI w;
    w.show();

    return a.exec();
}

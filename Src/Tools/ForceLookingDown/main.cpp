#include "force_looking_down_gui.h"

#include <QApplication>

int main(int argc, char *argv[])
{
    QApplication a(argc, argv);
    ForceLookingDownGUI w;
    w.show();
    return a.exec();
}

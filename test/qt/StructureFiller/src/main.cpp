#include <QScriptValue>

#include <QtGui/QApplication>
#include <QScriptEngine>

#include <QtDebug>
#include <QString>

#include "StructureFillerGui.h"


int main(int argc, char *argv[])
{
    QApplication a(argc, argv);
    StructureFillerGui w;




    w.show();
    


    return a.exec();
}

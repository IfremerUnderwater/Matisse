#include "mainwindow.h"
#include <QApplication>
#include <qgsapplication.h>
#include <QDebug>
#include <QProcessEnvironment>

int main(int argc, char *argv[])
{

    qDebug() << "Start";

     QProcessEnvironment env;

     QString oswgeo4w = env.systemEnvironment().value("OSGEO4W_ROOT");

    QgsApplication::setPrefixPath(oswgeo4w+"\\apps\\qgis", true);

#ifdef QT_DEBUG
    qDebug() << "Load Debug versions of plugins";
    QgsApplication::setPluginPath(oswgeo4w+"\\apps\\qgis\\pluginsd");
#endif
    QgsApplication::initQgis();


    qDebug() << QgsApplication::showSettings();



    QgsApplication a(argc, argv, true);

    MainWindow w;


    w.show();

    return a.exec();

}

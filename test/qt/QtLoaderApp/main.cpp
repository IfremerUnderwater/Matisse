#include <QCoreApplication>
#include <QPluginLoader>

#include <QtDebug>
#include "Processor.h"

using namespace MatisseCommon;

int main(int argc, char *argv[])
{
    QCoreApplication a(argc, argv);

    qDebug("QT APP");
    QString pluginDll = "../PluginMock/release/PluginMock.dll";

    QPluginLoader loader(pluginDll);

    if(QObject *pluginObject = loader.instance()) { // On prend l'instance de notre plugin sous forme de QObject. On verifie en meme temps s'il n'y a pas d'erreur.
        qDebug() << "Plugin Loaded" << endl;
        Processor* processor = qobject_cast<Processor *>(pluginObject);
        qDebug() << "Nom: " << processor->name();
        qDebug() << "Port de sortie: " << processor->outNumber();

    } else {
        qDebug() << "Unable to load plugin" << endl;
    }


    return a.exec();
}

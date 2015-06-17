#include "LoadPlugin.h"
#include "ui_LoadPlugin.h"
#include <QDebug>

LoadPlugin::LoadPlugin(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::LoadPlugin)
{
    ui->setupUi(this);
    _greenIcon = QIcon(":/ico/green-on.ico");
    _greenOffIcon = QIcon(":/ico/green-off.ico");
    _redIcon = QIcon(":/ico/red-on.ico");

    connect(ui->_PB_libs, SIGNAL(clicked()), this, SLOT(slot_showLibs()));
    connect(ui->_PB_unload, SIGNAL(clicked()), this, SLOT(slot_unloadAll()));
    connect(ui->_TAW_process, SIGNAL(cellClicked(int, int)), this, SLOT(slot_selectProcess(int, int)));

    QStringList labels = QString("Nom, Commentaires, Préfixe, Suffixe, Type, Représentation, Valeur courante, Valeur min, Valeur max, Null possible").split(", ");

    ui->_TAW_process->setColumnCount(2);
    ui->_TAW_process->setRowCount(0);
    ui->_TAW_process->setHorizontalHeaderLabels(QStringList() << "Nom" << "DLL");

    ui->_TAW_Params->setColumnCount(2);
    ui->_TAW_Params->setRowCount(0);
    ui->_TAW_Params->setHorizontalHeaderLabels(QStringList() << "Groupe" << "Param");
    // Chargement des plugins rep par defaut

}

LoadPlugin::~LoadPlugin()
{


    delete ui;
}

void LoadPlugin::load(QString libname) {
    QString ext = "";
#ifdef WIN32
    ext="*.dll";
#endif

    QDir pluginDir(libname);
    foreach (QString fileName, pluginDir.entryList(QStringList() << ext , QDir::Files)) {
        qDebug() << "Filename:" << fileName;
        QPluginLoader *loader = new QPluginLoader(pluginDir.absoluteFilePath(fileName));
        _plugins.insert(fileName, loader);
        if(QObject *pluginObject = loader->instance()) { // On prend l'instance de notre plugin sous forme de QObject. On vérifie en même temps s'il n'y a pas d'erreur.
            LifecycleComponent * newInstance = qobject_cast<LifecycleComponent *>(pluginObject); // On réinterprète alors notre QObject
            qDebug() << "Component" << newInstance->name() << " loaded.";
            _components.insert(newInstance->name(), newInstance);
            int row = ui->_TAW_process->rowCount();
              ui->_TAW_process->insertRow(row);

              QTableWidgetItem *item= new QTableWidgetItem ( newInstance->name());
              QTableWidgetItem *item1= new QTableWidgetItem ( fileName);
            ui->_TAW_process->setItem(row, 0, item);
            ui->_TAW_process->setItem(row, 1, item1);
        }
    }
}




void LoadPlugin::slot_selectProcess(int noRow, int noCol)
{
    Q_UNUSED(noCol)
    qDebug() << "Try to load:" << noRow;
    QString process = ui->_TAW_process->item(noRow, 0)->text();
    ui->_LE_pluginName->clear();
    LifecycleComponent* component = _components.value(process);
    QString dll = ui->_TAW_process->item(noRow, 1)->text();
    if(component) {
       ui->_LE_pluginName->setText(component->name());
       ui->_LE_pluginID->setText(dll);
      QList<MatisseParameter> params = component->expectedParameters();
      ui->_TAW_Params->clearContents();
      ui->_TAW_Params->setRowCount(0);
      foreach (MatisseParameter param, params) {
        QTableWidgetItem *item = new QTableWidgetItem(param.structure);
        QTableWidgetItem *item1 = new QTableWidgetItem(param.param);
        int row = ui->_TAW_Params->rowCount();
        ui->_TAW_Params->insertRow(row);
        ui->_TAW_Params->setItem(row, 0, item);
        ui->_TAW_Params->setItem(row, 1, item1);

      }

    } else {
        qDebug() << "Error: no component";
    }

}

void LoadPlugin::slot_showLibs()
{
    QString lib = ui->_LE_libs->text();
    qDebug() << "Show libs: " << lib;
    load(lib);
}

void LoadPlugin::slot_unloadAll()
{
    foreach(QPluginLoader * loader, _plugins.values()) {

        if (loader) {
            loader->unload();
            loader->deleteLater();
        }
    }
    ui->_TAW_process->clearContents();
    ui->_TAW_process->setRowCount(0);
    _components.clear();
    _plugins.clear();


}


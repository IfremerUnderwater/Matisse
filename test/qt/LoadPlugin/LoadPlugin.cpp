#include "LoadPlugin.h"
#include "ui_LoadPlugin.h"

LoadPlugin::LoadPlugin(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::LoadPlugin)
{
    ui->setupUi(this);
    _greenIcon = QIcon(":/ico/green-on.ico");
    _greenOffIcon = QIcon(":/ico/green-off.ico");
    _redIcon = QIcon(":/ico/red-on.ico");

    connect(ui->_PB_libs, SIGNAL(clicked()), this, SLOT(slot_showLibs()));
    connect(ui->_PB_process, SIGNAL(clicked()), this, SLOT(slot_startProcess()));
    connect(ui->_PB_customize, SIGNAL(clicked()), this, SLOT(slot_customize()));
    connect(ui->_TAW_process, SIGNAL(cellClicked(int, int)), this, SLOT(slot_selectProcess(int, int)));

    ui->_TAW_launchedProcesses->clear();
    ui->_TAW_launchedProcesses->setColumnCount(1);
    QStringList labels = QString("Nom, Commentaires, Préfixe, Suffixe, Type, Représentation, Valeur courante, Valeur min, Valeur max, Null possible").split(", ");
    ui->_TAW_pluginParameters->setColumnCount(labels.length());
    ui->_TAW_pluginParameters->setHorizontalHeaderLabels(labels);

    ui->_TAW_process->setColumnCount(2);
    ui->_TAW_process->setRowCount(0);
    ui->_TAW_process->setHorizontalHeaderLabels(QStringList() << "Nom" << "ID");

    // Chargement des plugins rep par defaut

}

LoadPlugin::~LoadPlugin()
{
    foreach(QPluginLoader * loader, _plugins.keys()) {
        PluginInterface * plugin = _plugins.value(loader);
        if (plugin && plugin->isRunning()) {
            plugin->terminate();
            plugin->deleteLater();
            delete plugin;
        }
        loader->unload();
        loader->deleteLater();
    }

    delete ui;
}

void LoadPlugin::load(QString libname) {
    QPluginLoader * loader = new QPluginLoader("../ProcessorExtractor/generatedPlugins/available/PLUGIN_da1c2536_2bc8_4e1e_bb83_b4ab14a34dd5.dll"); // On charge le plugin en lui donnant juste le chemin.
    qDebug() << "Try to load...";
    if(QObject *pluginObject = loader->instance()) { // On prend l'instance de notre plugin sous forme de QObject. On vérifie en même temps s'il n'y a pas d'erreur.
        qDebug() << "Loaded...";
        PluginInterface* plugin = qobject_cast<PluginInterface *>(pluginObject); // On réinterprète alors notre QObject en MyPlugin
        QList<QVariant> args;
        args << "Toto" << "Tutu" << "Titi";
        PluginInterface * newInstance = plugin->newInstance();
        connect(newInstance, SIGNAL(started()), this, SLOT(slot_processorStarted()));
        connect(newInstance, SIGNAL(finished()), this, SLOT(slot_processorEnded()));
        newInstance->runProcessor();
//        newInstance->runProcessor(args);
        _plugins.insert(loader, newInstance);
    } else {
        int noRow = ui->_TAW_launchedProcesses->rowCount();
        ui->_TAW_launchedProcesses->insertRow(noRow);
        QTableWidgetItem * newItem = new QTableWidgetItem(_redIcon, QString("Processor %1").arg(0));
        newItem->setData(Qt::UserRole, 0);
        ui->_TAW_launchedProcesses->setItem(noRow, 0, newItem);

        qDebug() << "Error:" << loader->errorString();
    }
}

void LoadPlugin::startProcess(QPluginLoader * loader) {
    if(QObject *pluginObject = loader->instance()) { // On prend l'instance de notre plugin sous forme de QObject. On vérifie en même temps s'il n'y a pas d'erreur.
        qDebug() << "Loaded...";
        PluginInterface* plugin = qobject_cast<PluginInterface *>(pluginObject); // On réinterprète alors notre QObject en MyPlugin
        QList<QVariant> args;
        args << "Toto" << "Tutu" << "Titi";
        PluginInterface * newInstance = plugin->newInstance();
        connect(newInstance, SIGNAL(started()), this, SLOT(slot_processorStarted()));
        connect(newInstance, SIGNAL(finished()), this, SLOT(slot_processorEnded()));
//        newInstance->runProcessor(args);
        newInstance->runProcessor();
        _plugins.insert(loader, newInstance);
    } else {
        int noRow = ui->_TAW_launchedProcesses->rowCount();
        ui->_TAW_launchedProcesses->insertRow(noRow);
        QTableWidgetItem * newItem = new QTableWidgetItem(_redIcon, QString("Processor %1").arg(0));
        newItem->setData(Qt::UserRole, 0);
        ui->_TAW_launchedProcesses->setItem(noRow, 0, newItem);

        qDebug() << "Error:" << loader->errorString();
    }

}

void LoadPlugin::slot_startProcess()
{
    load("toto");
}

void LoadPlugin::slot_processorStarted()
{
    int noRow = ui->_TAW_launchedProcesses->rowCount();
    ui->_TAW_launchedProcesses->insertRow(noRow);
    QTableWidgetItem * newItem = new QTableWidgetItem(_greenIcon, QString("Processor %1").arg((int)sender()));
    newItem->setData(Qt::UserRole, (int)sender());
    ui->_TAW_launchedProcesses->setItem(noRow, 0, newItem);
    //    ui->_TAW_launchedProcesses->setItem(noRow, 1, new QTableWidgetItem(QString("Processor %1").arg(sender())));
}

void LoadPlugin::slot_processorEnded()
{
    for(int noRow = 0; noRow < ui->_TAW_launchedProcesses->rowCount(); noRow++) {
        QObject * processId = (QObject *)ui->_TAW_launchedProcesses->item(noRow, 0)->data(Qt::UserRole).toInt();
        if (processId == sender()) {
            ui->_TAW_launchedProcesses->item(noRow, 0)->setIcon(_greenOffIcon);
            break;
        }
    }
}

void LoadPlugin::slot_selectProcess(int noRow, int noCol)
{
    qDebug() << "Try to load:" << noRow;
    QString process = ui->_TAW_process->item(noRow, 1)->text();
    QPluginLoader * loader = new QPluginLoader("../ProcessorExtractor/generatedPlugins/available/" + process); // On charge le plugin en lui donnant juste le chemin.
    ui->_LE_pluginName->clear();
    ui->_LE_pluginComments->clear();
    ui->_TAW_pluginParameters->clearContents();
    ui->_TAW_pluginParameters->setRowCount(0);
    if(QObject *pluginObject = loader->instance()) { // On prend l'instance de notre plugin sous forme de QObject. On vérifie en même temps s'il n'y a pas d'erreur.
        qDebug() << "Loaded...";
        PluginInterface* plugin = qobject_cast<PluginInterface *>(pluginObject); // On réinterprète alors notre QObject en MyPlugin
        ui->_LE_pluginName->setText(plugin->processorName());
        ui->_LE_pluginComments->setText(plugin->processorComments());
        ui->_LE_pluginID->setText(process);
        /*
        QList <ProcessorParameter> parameters = plugin->parameters();
        foreach(ProcessorParameter parameter, parameters) {
            int noRow = ui->_TAW_pluginParameters->rowCount();

            ui->_TAW_pluginParameters->insertRow(noRow);
            ui->_TAW_pluginParameters->setItem(noRow, 0, new QTableWidgetItem(parameter._name));
            ui->_TAW_pluginParameters->setItem(noRow, 1, new QTableWidgetItem(parameter._comments));
            ui->_TAW_pluginParameters->setItem(noRow, 2, new QTableWidgetItem(parameter._prefix));
            ui->_TAW_pluginParameters->setItem(noRow, 3, new QTableWidgetItem(parameter._suffix));
            ui->_TAW_pluginParameters->setItem(noRow, 4, new QTableWidgetItem(parameter._type));
            ui->_TAW_pluginParameters->setItem(noRow, 5, new QTableWidgetItem(parameter._show));
            ui->_TAW_pluginParameters->setItem(noRow, 6, new QTableWidgetItem(parameter._value.toString()));
            ui->_TAW_pluginParameters->setItem(noRow, 7, new QTableWidgetItem(parameter._minValue.toString()));
            ui->_TAW_pluginParameters->setItem(noRow, 8, new QTableWidgetItem(parameter._maxValue.toString()));
            ui->_TAW_pluginParameters->setItem(noRow, 9, new QTableWidgetItem(parameter._required));

        }
        */
        loader->unload();
        loader->deleteLater();

    } else {
        qDebug() << "Error:" << loader->errorString();
    }

}

void LoadPlugin::slot_showLibs()
{
    qDebug() << "Show Libs...";
    QString selectedDir = QFileDialog::getExistingDirectory(this, "Répertoire des plugins", QDir::currentPath());
    if (selectedDir == ""){
        qDebug() << "Return...";
        return;
    }
    _libsDir = selectedDir;
    ui->_TAW_process->clear();
    QDir dir(selectedDir);
    qDebug() << "Selected dir=" << dir.absolutePath();
    dir.setNameFilters(QStringList() << "*.dll");
    QStringList plugins = dir.entryList();
    qDebug() << "Length=" << plugins.length();

    // recupération des noms des process dans les plugins
    foreach(QString plugin, plugins) {
        qDebug() << "Try to load:" << plugin;
        QPluginLoader * loader = new QPluginLoader(_libsDir + "/" + plugin); // On charge le plugin...
        if(QObject *pluginObject = loader->instance()) {
            // On prend l'instance de notre plugin sous forme de QObject. On vérifie en même temps s'il n'y a pas d'erreur.
            qDebug() << "Loaded...";
            PluginInterface* pluginInterface = qobject_cast<PluginInterface *>(pluginObject); // On réinterprète alors notre QObject en MyPlugin
            qDebug() << "Processor name=" << pluginInterface->processorName();
            int noRow = ui->_TAW_process->rowCount();
            ui->_TAW_process->insertRow(noRow);
            ui->_TAW_process->setItem(noRow, 0, new QTableWidgetItem(pluginInterface->processorName()));
            ui->_TAW_process->setItem(noRow, 1, new QTableWidgetItem(plugin));
            loader->unload();
            loader->deleteLater();
        }

    }
    ui->_TAW_process->resizeColumnsToContents();

}

void LoadPlugin::slot_customize()
{
    // recuperation parametres
    QString process = ui->_LE_pluginID->text();
    QPluginLoader * loader = new QPluginLoader("../ProcessorExtractor/generatedPlugins/available/" + process); // On charge le plugin en lui donnant juste le chemin.

    QObject *pluginObject = loader->instance();
    if (!pluginObject) {
        loader->unload();
        loader->deleteLater();

        return;
    }
    // On prend l'instance de notre plugin sous forme de QObject. On vérifie en même temps s'il n'y a pas d'erreur.
    qDebug() << "Loaded...";
    // on cree l'interface pour test....
    QDialog pluginDialog(this);

    QVBoxLayout * boxLayout = new QVBoxLayout();


    PluginInterface* plugin = qobject_cast<PluginInterface *>(pluginObject); // On réinterprète alors notre QObject en MyPlugin
 /*
    QList <ProcessorParameter> parameters = plugin->parameters();
    foreach(ProcessorParameter parameter, parameters) {

        qDebug() << "NOM:" << parameter._name;
        qDebug() << "COMMENTS:" << parameter._comments;
        qDebug() << "PREFIX:" << parameter._prefix;
        qDebug() << "SUFFIX:" << parameter._suffix;
        qDebug() << "TYPE:" << parameter._type;
        qDebug() << "SHOW:" << parameter._show;
        qDebug() << "VALUE:" << parameter._value.toString();
        qDebug() << "MIN:" << parameter._minValue.toString();
        qDebug() << "MAX:" << parameter._maxValue.toString();
        qDebug() << "REQUIRED:" << parameter._required;

        QHBoxLayout * parameterLayout = new QHBoxLayout();
        QWidget * widget = 0;
        switch (parameter._show) {
        case LINE_EDIT: {
            qDebug() << "Line edit";
            widget = new QLineEdit();
        }
            break;
        case COMBO_BOX: {
            widget = new QComboBox();
            // remplissage combo....
            // test enum....
            qDebug() << "Traite enum Combo";
            switch (parameter._type) {
            case INT: {

            }
                break;
            case REAL: {

            }
                break;
            case STRING: {

            }
                break;
            case BOOL: {

            }
                break;
            case ENUM: {

            }
                break;
            }
        }
            break;
        case SPIN_BOX: {
            widget = new QSpinBox();
        }
            break;
        case SLIDER: {
            widget = new QSlider(Qt::Horizontal);
        }
            break;
        case FILE_SELECTOR: {
            widget = new ChooseAndShow("File");
            connect(((ChooseAndShow *)widget), SIGNAL(signal_clicked()), this, SLOT(slot_openFileDialog()));
        }
            break;
        case DIR_SELECTOR: {
            widget = new ChooseAndShow("Dir");
            connect(((ChooseAndShow *)widget), SIGNAL(signal_clicked()), this, SLOT(slot_openFileDialog()));
        }
            break;
        case CHECK_BOX: {
            widget = new QCheckBox();
        }
            break;
        }
        qDebug() << "Add param...";
        parameterLayout->addWidget(new QLabel(parameter._name + ":"));
        parameterLayout->addWidget(widget);
        parameterLayout->addStretch();
        //groupBoxLayout->addLayout(parameterLayout);
        boxLayout->addLayout(parameterLayout);

    }
    loader->unload();
    loader->deleteLater();

    boxLayout->addStretch();
    pluginDialog.setLayout(boxLayout);
    pluginDialog.exec();
*/
}

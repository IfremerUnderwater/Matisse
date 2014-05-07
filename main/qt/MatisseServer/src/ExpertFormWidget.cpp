#include "ExpertFormWidget.h"
#include "ui_ExpertFormWidget.h"

using namespace MatisseServer;

ExpertFormWidget::ExpertFormWidget(Server *  server, QWidget *parent) :
    QWidget(parent),
    _ui(new Ui::ExpertFormWidget),
    _server(server),
    //_rootXml(rootXml),
    //_xmlTool(rootXml),
    _currentParameters(NULL)
{
    _ui->setupUi(this);
    init();
}

ExpertFormWidget::~ExpertFormWidget()
{
    delete _ui;
}

void ExpertFormWidget::addProcessorWidget(ProcessorWidget * procWidget)
{
    QListWidgetItem * newProcItem = new QListWidgetItem(procWidget->getIcon(), procWidget->getName());
    newProcItem->setData(Qt::UserRole, qlonglong(procWidget));
    _availableProcessors.insert(procWidget->getName(), procWidget);
    _ui->_LW_processors->addItem(newProcItem);
}

void ExpertFormWidget::addSourceWidget(SourceWidget *sourceWidget)
{
    QListWidgetItem * newSrcItem = new QListWidgetItem(sourceWidget->getIcon(), sourceWidget->getName());
    newSrcItem->setData(Qt::UserRole, qlonglong(sourceWidget));
    _availableSources.insert(sourceWidget->getName(), sourceWidget);
    _ui->_LW_inputs->addItem(newSrcItem);
}

void ExpertFormWidget::addDestinationWidget(DestinationWidget *destinationWidget)
{
    QListWidgetItem * newDestItem = new QListWidgetItem(destinationWidget->getIcon(), destinationWidget->getName());
    newDestItem->setData(Qt::UserRole, qlonglong(destinationWidget));
    _availableDestinations.insert(destinationWidget->getName(), destinationWidget);
    _ui->_LW_outputs->addItem(newDestItem);
}

void ExpertFormWidget::init()
{
    // _ui->_GRW_assembly->resize(200, 400);
    // _ui->_SPL_elements->resize(200, 400);
    // _ui->_SPL_assembly->setStretchFactor(0, 3);
    // _ui->_SPL_assembly->setStretchFactor(1, 2);
    // _ui->_SPL_elements->setStretchFactor(0, 1);
    // _ui->_SPL_elements->setStretchFactor(1, 1);

    _lastUsedParameter = NULL;

    connect(_ui->_LW_inputs, SIGNAL(clicked(QModelIndex)), this, SLOT(slot_selectElement(QModelIndex)));
    connect(_ui->_LW_processors, SIGNAL(clicked(QModelIndex)), this, SLOT(slot_selectElement(QModelIndex)));
    connect(_ui->_LW_outputs, SIGNAL(clicked(QModelIndex)), this, SLOT(slot_selectElement(QModelIndex)));

    //connect(_ui->_TRW_parameters, SIGNAL(itemClicked(QTreeWidgetItem*,int)), this, SLOT(slot_showParameters(QTreeWidgetItem*,int)));
    connect(_ui->_TRW_parameters, SIGNAL(itemPressed(QTreeWidgetItem*,int)), this, SLOT(slot_showParameters(QTreeWidgetItem*,int)));
    connect(_ui->_TAB_elements, SIGNAL(currentChanged(int)), this, SLOT(slot_changeTabPanel(int)));

    connect(_ui->_LW_inputs, SIGNAL(doubleClicked(QModelIndex)), this, SLOT(slot_showElement(QModelIndex)));
    connect(_ui->_LW_processors, SIGNAL(doubleClicked(QModelIndex)), this, SLOT(slot_showElement(QModelIndex)));
    connect(_ui->_LW_outputs, SIGNAL(doubleClicked(QModelIndex)), this, SLOT(slot_showElement(QModelIndex)));

    // connect(_ui->_PB_saveParameters, SIGNAL(clicked()), this, SLOT(slot_saveParameters()));

    _ui->_GRW_assembly->setAcceptDrops(true);
    _scene = new AssemblyGraphicsScene(_server, _ui->_GRW_assembly->rect());
    _ui->_GRW_assembly->setScene(_scene);
    _ui->_GRW_assembly->centerOn(0, 0);
    _scene->setExpertGui(this);
    // _parameters = new Tools();
    //  if (!_parameters->readParametersFile(_rootXml + "/models/Parameters.xml")) {
    //      QMessageBox::warning(this, "Erreur de fichier 2", "Le fichier n'est pas au format attendu " + _rootXml + "/models/Parameters.xml");
    //      return;
    //  }

    //   _ui->_SCA_element->setWidget(_parameters->createFullParametersDialog());

    // test();
    fillLists();
}

void ExpertFormWidget::fillLists()
{
    _lastUsedParameter = NULL;
    // chargement des parameteres
    // on recupere les fichiers et on verifie la compatibilité avec les modeles...
    QString rootXml = _server->xmlTool().getBasePath();
    QDir rootParametersDir(rootXml + "/parameters");
    if (!rootParametersDir.exists()) {
        // erreur
        return;
    }
    // creation de l'arborescence
    QStringList parametersDir = rootParametersDir.entryList(QDir::Dirs | QDir::NoDotAndDotDot | QDir::Readable);
    QStringList versions;
    QRegExp versionExp("^V\\d(.\\d)+$");
    _availableParameters.clear();
    foreach(QString dirname, parametersDir) {
        if (!versionExp.exactMatch(dirname)) {
            continue;
        }
        rootParametersDir.cd(dirname);
        QStringList parametersFiles = rootParametersDir.entryList(QStringList() <<"*.xml", QDir::Files | QDir::Readable);
        foreach (QString filename, parametersFiles) {
            filename = rootParametersDir.absoluteFilePath(filename);
            qDebug() << "Fichier parametres:" << filename;
            KeyValueList params = _server->xmlTool().readParametersFileDescriptor(filename);
            QString version = params.getValue("modelVersion");
            if (version == dirname) {
                QString name = params.getValue("name");
                ParametersWidget * newParameters = new ParametersWidget();
                int versionIndex;
                if ((versionIndex = versions.indexOf(version)) > -1) {
                    newParameters->setName(version + "\n" + name);
                    QTreeWidgetItem * newParamItem = new QTreeWidgetItem(_ui->_TRW_parameters->topLevelItem(versionIndex), QStringList() << name);
                    newParamItem->setData(0, Qt::UserRole, version);
                    newParamItem->setIcon(0, newParameters->getIcon());
                    _availableParameters.insert(version + "/" + name, newParameters);
                } else {
                    newParameters->setName(version);
                    QTreeWidgetItem * newParamVersion = new QTreeWidgetItem(QStringList() << version);
                    newParamVersion -> setData(0, Qt::UserRole, version);
                    newParamVersion -> setIcon(0, newParameters->getIcon());
                    _ui->_TRW_parameters->addTopLevelItem(newParamVersion);
                    _availableParameters.insert(version, newParameters);
                    versions << version;

                    newParameters = new ParametersWidget();
                    newParameters->setName(version + "\n" + name);
                    QTreeWidgetItem * newParamItem = new QTreeWidgetItem(newParamVersion, QStringList() << name);
                    newParamItem->setData(0, Qt::UserRole, version);
                    newParamItem->setIcon(0, newParameters->getIcon());
                    _availableParameters.insert(version + "/" + name, newParameters);
                }
            }

        }
    }

    _ui->_TRW_parameters->expandAll();
}

void ExpertFormWidget::slot_showParameters(QTreeWidgetItem*item, int noCol)
{
    QString modelVersionStr;
    QString parametersNameStr;
    _lastUsedParameter = NULL;
    if (item) {
        _lastUsedParameter = item;
        qDebug() << "Show parameters";
        QString rootXml = _server->xmlTool().getBasePath();

        // on teste si des parameteres ont été modifiés
        if (_currentParameters) {
             ParametersWidgetSkeleton * paramWidget  = qobject_cast<ParametersWidgetSkeleton *>(_ui->_SCA_element->widget());
             if (paramWidget) {
                 if (paramWidget->hasModifiedValues() && (noCol>-1)) {
                     if (QMessageBox::Yes ==QMessageBox::question(this, "Paramètres modifiés...", "Voulez vous enregistrer les paramètres?"
                                                                  , QMessageBox::Yes, QMessageBox::No)) {
                         emit signal_saveParameters();
                     }
                 }
             }
            delete _currentParameters;
        }
        _currentParameters = new Tools();

        if (item->parent() == NULL) {
            // on a un modele...
            qDebug() << "Chargement modèle de paramètres...";
            modelVersionStr = item->data(0, Qt::DisplayRole).toString();
            QString modelName = rootXml
                    + QDir::separator() + "models"
                    + QDir::separator() + "parameters"
                    + QDir::separator() + "Parameters_" + item->data(0, Qt::DisplayRole).toString().replace(" ", "_") + ".xml";
            if (!_currentParameters -> readParametersModelFile(modelName)) {
                QMessageBox::warning(this, "Fichier modèle de paramètres", "Le fichier modèle\n" + modelName + "\nn'est pas au format attendu ");
                return;
            }

        } else {
            // on a un jeu de parametres...
            qDebug() << "Chargement jeu de paramètres...";
            modelVersionStr = item->parent()-> data(0, Qt::DisplayRole).toString();
            parametersNameStr =  item->data(0, Qt::DisplayRole).toString();
            QString modelVersion = item->parent()-> data(0, Qt::DisplayRole).toString().trimmed().replace(" ", "_");
            QString modelName = rootXml
                    + QDir::separator() + "models"
                    + QDir::separator() + "parameters"
                    + QDir::separator() + "Parameters_" + modelVersion + ".xml";
            QString filename = rootXml
                    + QDir::separator() + "parameters"
                    + QDir::separator() + modelVersion
                    + QDir::separator() + item->data(0, Qt::DisplayRole).toString().replace(" ", "_") + ".xml";
            if (!_currentParameters -> readUserParametersFile(filename, modelName)) {
                QMessageBox::warning(this, "Fichier de paramètres", "Le fichier\n" + filename + "\nn'est pas au format attendu ");
                return;
            }
        }

        ParametersWidgetSkeleton * paramWidget = _currentParameters -> createFullParametersDialog();
        qDebug() << "Connect signal..." << connect(paramWidget, SIGNAL(signal_valuesModified(bool)), this, SLOT(slot_parametersValuesModified(bool)));
        _ui->_SCA_element->setWidget(paramWidget);
    } else {
        if (_currentParameters) {
            delete _currentParameters;
            _currentParameters = NULL;
        }
        _ui->_SCA_element->setWidget(new QWidget());
    }
    QString groupBoxTitle = "Paramètres courants " + modelVersionStr;
    if (!parametersNameStr.isEmpty()) {
        groupBoxTitle.append("/" + parametersNameStr);
    }
    _ui->_GB_parameters->setTitle(groupBoxTitle);
}

void ExpertFormWidget::slot_parametersValuesModified(bool modified)
{
    // sert uniquement a reemettre un signal...
    qDebug() << "emit signal 2";
    emit signal_parametersValuesModified(modified);

}

void ExpertFormWidget::slot_changeTabPanel(int panel)
{

}

bool ExpertFormWidget::saveParameters()
{
    // recuperation nom fichier xml
    qDebug() << "Save parameters";
    KeyValueList fields;
    QTreeWidgetItem * currentParams = _ui->_TRW_parameters->currentItem();
    if (!currentParams) {
        // TODO: add message
        return false;
    }
    QString version = currentParams->data(0,Qt::UserRole).toString();
    fields.set("modelVersion", version);

    QString rootXml = _server->xmlTool().getBasePath();

    ParametersDialog * parametersDialog = new ParametersDialog(this, rootXml + "/parameters/" + version, fields);

    bool ret = false;

    if (parametersDialog->exec() == QDialog::Accepted) {
        // ecriture fichier
        QString filename = parametersDialog->getFilename();
        fields = parametersDialog->getFields();
        qDebug() << "Ecriture" << filename;
        _currentParameters->generateParametersFile(filename, fields);
        // ajout dans la liste des parametres...
        QString name = fields.getValue("name");
        ParametersWidget * newParametersWidget = new ParametersWidget();
        newParametersWidget->setName(version + "\n" + name);
        QTreeWidgetItem * newParamsItem = new QTreeWidgetItem(QStringList() << name);
        newParamsItem->setData(0, Qt::UserRole, version);
        newParamsItem->setIcon(0, newParametersWidget->getIcon());

        _availableParameters.insert(version + "/" + name, newParametersWidget);

        _lastUsedParameter = newParamsItem;
        _ui->_TRW_parameters->setCurrentItem(_lastUsedParameter);

        if (currentParams->parent() == 0) {
            currentParams->addChild(newParamsItem);
        } else {
            currentParams->parent()->addChild(newParamsItem);
        }
        ret = true;
    }

    parametersDialog->deleteLater();

    return ret;
}

void ExpertFormWidget::selectLastUsedParameter()
{
    slot_showParameters(_lastUsedParameter, -1);
}

bool ExpertFormWidget::selectParametersItem(QString model, QString parameters)
{
    bool ret = false;
    for(int noModel = 0; noModel < _ui->_TRW_parameters->topLevelItemCount(); noModel++) {
        QTreeWidgetItem * modelItem = _ui->_TRW_parameters->topLevelItem(noModel);
        if (modelItem->text(0) == model) {
            for (int noParameters = 0; noParameters < modelItem->childCount(); noParameters++) {
                QTreeWidgetItem * paramsItem = modelItem->child(noParameters);
                if (paramsItem->text(0) == parameters) {
                    _ui->_TRW_parameters->setCurrentItem(paramsItem);
                    ret = true;
                    break;
                }
            }
            break;
        }
    }

    return ret;
}

void ExpertFormWidget::test() {

    ProcessorWidget * newProc = new ProcessorWidget();
    newProc->setName("Processeur 1");
    newProc->setInputsNumber(2);
    newProc->setOutputsNumber(4);

    ProcessorWidget * newProc2 = new ProcessorWidget();
    newProc2->setName("Processeur 2");
    newProc2->setInputsNumber(1);
    newProc2->setOutputsNumber(2);

    ProcessorWidget * newProc3 = new ProcessorWidget();
    newProc3->setName("Doubleur de voies");
    newProc3->setInputsNumber(1);
    newProc3->setOutputsNumber(2);

    SourceWidget * newSource = new SourceWidget();
    newSource->setName("Source 1");
    newSource->setOutputsNumber(2);
    _availableSources.insert(newSource->getName(), newSource);
    QListWidgetItem * newSrcItem = new QListWidgetItem(newSource->getIcon(), newSource->getName());
    _ui->_LW_inputs->addItem(newSrcItem);

    DestinationWidget * newDest = new DestinationWidget();
    newDest->setName("Destination 1");
    newDest->setInputsNumber(2);

    ParametersWidget * newParamsModel = new ParametersWidget();
    newParamsModel->setName("V1.0");

    ParametersWidget * newParameters = new ParametersWidget();
    newParameters->setName("Parametres 1");

    //    _availableParameters.insert(newParamsModel->getName(), newParamsModel);
    //    _availableParameters.insert(newParamsModel->getName() + "/" + newParameters->getName(), newParameters);

    _availableProcessors.insert("Processeur 1", newProc);
    _availableProcessors.insert("Processeur 2", newProc2);
    _availableProcessors.insert("Doubleur de voies", newProc3);
    _availableDestinations.insert("Destination 1", newDest);

    QListWidgetItem * newItemIcon1 = new QListWidgetItem(newProc->getIcon(), newProc->getName());
    _ui->_LW_processors->addItem(newItemIcon1);
    newItemIcon1->setData(Qt::UserRole, qlonglong(newProc));
    QListWidgetItem * newItemIcon2 = new QListWidgetItem(newProc2->getIcon(), newProc2->getName());
    _ui->_LW_processors->addItem(newItemIcon2);
    newItemIcon1->setData(Qt::UserRole, qlonglong(newProc2));

    QListWidgetItem * newItemSplit = new QListWidgetItem(newProc3->getIcon(), newProc3->getName());
    _ui->_LW_processors->addItem(newItemSplit);
    newItemIcon1->setData(Qt::UserRole, qlonglong(newProc3));



    QListWidgetItem * newDestItem = new QListWidgetItem(newDest->getIcon(), newDest->getName());
    _ui->_LW_outputs->addItem(newDestItem);

    // test xml
    //qDebug() << "Validation schema";
    //Xml::validateXmlFile("../xml/models/parameters/Parameters_V1.0.xsd");
    //qDebug() << "Validation Fichier";
    //Xml::validateXmlFile("../xml/models/parameters/Parameters_V1.0.xsd", "../xml/parameters/Parametres_1.xml");
    //MatisseXml matisseXml("../xml");
    //matisseXml.readParametersFileDescriptor("test2.xml");
}

void ExpertFormWidget::showParameters(AssemblyDefinition *assembly)
{
    QTreeWidgetItem parent;
    QTreeWidgetItem item(&parent);
    parent.setData(0, Qt::DisplayRole, assembly->parametersDefinition()->model());
    item.setData(0, Qt::DisplayRole, assembly->parametersDefinition()->name());
    // selection de l'item...
    if (selectParametersItem(assembly->parametersDefinition()->model(), assembly->parametersDefinition()->name())) {
        slot_showParameters(&item, 0);
    } else {
        // parametres invalides...
        slot_showParameters(NULL, 0);
        // on deselectionne
        _ui->_TRW_parameters->clearSelection();

    }
}

void ExpertFormWidget::showParameters(QString parametersName)
{

}


ParametersWidget *ExpertFormWidget::getParametersWidget(QString name)
{
    qDebug() << "Recuperation param" << name.replace("_", " ");
    ParametersWidget * wid = _availableParameters.value(name, 0);


    if (!wid) {
        qWarning() << "Paramètres non trouvés";
        return 0;
    }

    ParametersWidget * newWidget = new ParametersWidget();

    newWidget->clone(wid);

    return newWidget;
}

SourceWidget *ExpertFormWidget::getSourceWidget(QString name)
{
    SourceWidget * wid = _availableSources.value(name, 0);

    if (!wid) {
        return 0;
    }

    SourceWidget * newWidget = new SourceWidget();

    newWidget->clone(wid);

    return newWidget;
}

ProcessorWidget *ExpertFormWidget::getProcessorWidget(QString name)
{
    ProcessorWidget * wid = _availableProcessors.value(name, 0);

    if (!wid) {
        return 0;
    }

    ProcessorWidget * newWidget = new ProcessorWidget();

    newWidget->clone(wid);

    return newWidget;
}

DestinationWidget *ExpertFormWidget::getDestinationWidget(QString name)
{
    DestinationWidget * wid = _availableDestinations.value(name, 0);

    if (!wid) {
        return 0;
    }

    DestinationWidget * newWidget = new DestinationWidget();

    newWidget->clone(wid);

    return newWidget;
}

bool ExpertFormWidget::loadAssembly(QString assemblyName)
{
    qDebug() << "Load assembly:" << assemblyName;
    return _scene->loadAssembly(assemblyName);

}

bool ExpertFormWidget::saveAssembly(QString assemblyName, KeyValueList fields)
{
    QString filename = assemblyName.replace(" ", "_");
    return _scene->saveAssembly(_server->xmlTool().getAssembliesPath() + QDir::separator() + filename + ".xml", assemblyName, fields);
}

void ExpertFormWidget::resetAssemblyForm()
{
    _scene->reset();
    _ui->_GRW_assembly->invalidateScene();
    _ui->_GRW_assembly->update();
    // TODO: effacer le reste....
    // reset parametres
    slot_showParameters();
    // selection premier onglet
    _ui->_TAB_elements->setCurrentIndex(0);
    // deselection dans les onglets
    _ui->_TRW_parameters->clearSelection();
    _ui->_LW_inputs->clearSelection();
    _ui->_LW_processors->clearSelection();
    _ui->_LW_outputs->clearSelection();
}

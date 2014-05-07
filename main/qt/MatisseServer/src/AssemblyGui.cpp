#include "AssemblyGui.h"
#include "ui_AssemblyGui.h"

using namespace MatisseServer;

AssemblyGui::AssemblyGui(QString settingsFile, QWidget *parent) :
    QMainWindow(parent),
    _ui(new Ui::AssemblyGui)
{
    _ui->setupUi(this);
    _canShow = setSettingsFile(settingsFile);
    init();
    test();
}

AssemblyGui::~AssemblyGui()
{
    delete _ui;
}

bool AssemblyGui::setSettingsFile(QString settings)
{
    _settingsFile = settings;
    QString standardFile = "./MatisseSettings.xml";
    // lecture du fichier xml de settings
    if (_settingsFile == "") {
        _settingsFile = standardFile;
    }
    QFileInfo settingsFile(_settingsFile);
    if (!settingsFile.exists()) {
        QMessageBox::critical(this, "Fichier de configuration introuvable", "Impossible de trouver le fichier:\n" + _settingsFile + "\nRelancez l'application avec un nom de fichier valide en paramètre\nou un fichier " + standardFile + " valide!");
        return false;
    }

    if (!settingsFile.isReadable()) {
        QMessageBox::critical(this, "Fichier de configuration illisible", "Impossible de lire le fichier:\n" + _settingsFile + "\nRelancez l'application avec un nom de fichier lisible en paramètre\nou rendez le fichier " + standardFile + " lisible!");
        return false;
    }

    Xml xmlProcessing;
    xmlProcessing.readMatisseGuiSettings(_settingsFile);
    if ((_rootXml = xmlProcessing.getBasePath()) == "") {
        QMessageBox::critical(this, "Fichier de configuration incorrect", "La valeur de XmlRootDir ne peut être déterminée.\nRelancez l'application avec un paramètre XmlRootDir valide\ndans le fichier de configuration!");
        return false;
    }

    if ((_dataPath = xmlProcessing.getDataPath()) == "") {
        QMessageBox::critical(this, "Fichier de configuration incorrect", "La valeur de DataPath ne peut être déterminée.\nRelancez l'application avec un paramètre XmlRootDir valide\ndans le fichier de configuration!");
        return false;
    }

    _parameters = new Tools();
    return true;
}

bool AssemblyGui::isShowable()
{
    return _canShow;
}

void AssemblyGui::init()
{
    _userParameterModified = false;
    _expertValuesModified = false;

    _userFormWidget = _ui->_WID_userFrom;
    connect(_userFormWidget, SIGNAL(signal_parametersChanged(bool)), this, SLOT(slot_modifiedParameters(bool)));
    _userFormWidget->setTools(_parameters);

    //    connect(_ui->_TRW_assemblies, SIGNAL(itemClicked(QTreeWidgetItem *, int)), this, SLOT(slot_firstAssemblySelect(QTreeWidgetItem *, int)));
    //connect(_ui->_TRW_assemblies, SIGNAL(doubleClicked(QModelIndex)), this, SLOT(slot_showAssembly(QModelIndex)));
    connect(_ui->_TRW_assemblies, SIGNAL(itemClicked(QTreeWidgetItem*,int)), this, SLOT(slot_selectAssemblyOrJob(QTreeWidgetItem*,int)));

    connect(_ui->_ACT_deleteAssembly, SIGNAL(triggered()), this, SLOT(slot_deleteAssembly()));
    connect(_ui->_ACT_newAssembly, SIGNAL(triggered()), this, SLOT(slot_clearAssembly()));
    connect(_ui->_ACT_saveAssembly, SIGNAL(triggered()), this, SLOT(slot_saveAssembly()));
    connect(_ui->_ACT_saveAsAssembly, SIGNAL(triggered()), this, SLOT(slot_saveAsAssembly()));
    connect(_ui->_ACT_saveParameters, SIGNAL(triggered()), this, SLOT(slot_saveParameters()));

    connect(_ui->_ACT_saveJob, SIGNAL(triggered()), this, SLOT(slot_saveJob()));
    connect(_ui->_ACT_saveAsJob, SIGNAL(triggered()), this, SLOT(slot_saveAsJob()));
    connect(_ui->_ACT_deleteJob, SIGNAL(triggered()), this, SLOT(slot_deleteJob()));
    connect(_ui->_ACT_launchJob, SIGNAL(triggered()), this, SLOT(slot_launchJob()));

    connect(_ui->_ACT_userExpert, SIGNAL(triggered()), this, SLOT(slot_swapUserOrExpert()));
    connect(_ui->_ACT_reloadAssembliesAndJobs, SIGNAL(triggered()), this, SLOT(slot_assembliesReload()));

    //   connect(_userFormWidget->parametersWidget(), SIGNAL(signal_valuesModified(bool)), this, SLOT(slot_modifiedParameters(bool)));


    // ajout du spacer pour le bouton de lancement du process
    //    QWidget * spacer1 = new QWidget();
    //    QWidget * spacer2 = new QWidget();
    //    QWidget * spacer3 = new QWidget();
    //    spacer1->setMinimumWidth(100);
    //    spacer1->setMaximumWidth(100);
    //    spacer3->setMinimumWidth(100);
    //    spacer3->setMaximumWidth(100);

    // //   spacer1-> setSizePolicy(QSizePolicy::Fixed, QSizePolicy::Expanding);
    // //   spacer2-> setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
    //    spacer3-> setSizePolicy(QSizePolicy::Fixed, QSizePolicy::Expanding);
    //    _ui->_TOO_actions->insertWidget(_ui->_ACT_launchProcess, spacer1);
    //    _ui->_TOO_actions->insertWidget(_ui->_ACT_saveParameters, spacer2);
    //    _ui->_TOO_actions->addWidget(spacer3);


    _server.setSettingsFile();
    _server.init();

    _beforeSelect = true;
    _lastJobLaunchedItem = NULL;

    connect(&_server, SIGNAL(signal_jobProcessed(QString)), this, SLOT(slot_jobProcessed(QString)));

    loadAssembliesAndJobsLists();

    _expertFormWidget = new ExpertFormWidget(&_server);
    _ui->_SPL_userExpert->insertWidget(1, _expertFormWidget);
    _expertFormWidget->sizePolicy().setHorizontalStretch(_userFormWidget->sizePolicy().horizontalStretch());
    _expertFormWidget->hide();
    _expertFormWidget->getScene()->setMainGui(this);
    connect(_expertFormWidget, SIGNAL(signal_parametersValuesModified(bool)), this, SLOT(slot_modifiedParameters(bool)));
    connect(_expertFormWidget, SIGNAL(signal_saveParameters()), this, SLOT(slot_saveParameters()));

    // Loading processors
    qDebug() << "Available processors " << _server.getAvailableProcessors().size();
    foreach (Processor * processor, _server.getAvailableProcessors()) {
        qDebug() << "Add processor " << processor->name();
        ProcessorWidget * procWidget = new ProcessorWidget();
        procWidget->setName(processor->name());
        procWidget->setInputsNumber(processor->inNumber());
        procWidget->setOutputsNumber(processor->outNumber());
        _expertFormWidget->addProcessorWidget(procWidget);
    }


    qDebug() << "Available ImageProviders " << _server.getAvailableImageProviders().size();
    foreach (ImageProvider * imageProvider, _server.getAvailableImageProviders()) {
        qDebug() << "Add imageProvider " << imageProvider->name();
        SourceWidget * srcWidget = new SourceWidget();
        srcWidget->setName(imageProvider->name());
        srcWidget->setOutputsNumber(imageProvider->outNumber());
        _expertFormWidget->addSourceWidget(srcWidget);
    }

    qDebug() << "Available RasterProviders " << _server.getAvailableRasterProviders().size();
    foreach (RasterProvider * rasterProvider, _server.getAvailableRasterProviders()) {
        qDebug() << "Add rasterProvider " << rasterProvider->name();
        DestinationWidget * destWidget = new DestinationWidget();
        destWidget->setName(rasterProvider->name());
        destWidget->setInputsNumber(rasterProvider->inNumber());
        _expertFormWidget->addDestinationWidget(destWidget);
    }

    slot_swapUserOrExpert();
    _userFormWidget->createCanvas();

    // etat intial actions
    setActionsStates();
    //statusBar
    initStatusBar();

}

void AssemblyGui::test()
{
    // Pour test
    // Lecture fichier dim2
    // Dim2FileReader reader("C:/WorkspaceMatisse/Test_dataset/otus.dim2");

}

// TODO Chargement des jobs à déplacer dans Server ou Xml
void AssemblyGui::loadAssembliesAndJobsLists()
{
    // parse du repertoire et recuperation des differents jobs
    //_assembliesValues.clear();
    _currentJobName="";
    QString jobsPath =  _server.xmlTool().getJobsPath();
    QString assembliesPath = _server.xmlTool().getAssembliesPath();

    qDebug() << "Load assemblies from " + assembliesPath;
    QDir assembliesDir(assembliesPath);
    QStringList assemblies = assembliesDir.entryList(QStringList() << "*.xml");
    _server.xmlTool().clearAssembliesDatas();

    // La liste des topItems
    _ui->_TRW_assemblies->clear();
    _assembliesItems.clear();

    // chargement des assemblies
    foreach(QString assembly, assemblies) {
        qDebug() << "Lecture assembly" << assembly;
        if (!_server.xmlTool().readAssemblyFile(assembly)) {
            qDebug() << "Unable to read assembly " << assembly;
            continue;
        }
    }

    // deuxième passe, création des items...
    // on met le nom du fichier dans le userRole afin
    // de le rendre accessible ailleurs...
    foreach(QString assemblyName, _server.xmlTool().getAssembliesList()) {
        addAssemblyInTree(_server.xmlTool().getAssembly(assemblyName));
    }

    qDebug() << "Load jobs from " + jobsPath;
    QDir jobsDir(jobsPath);
    QStringList jobs  = jobsDir.entryList(QStringList() << "*.xml");
    _server.xmlTool().clearJobsDatas();
    foreach(QString job, jobs) {
        //qDebug() << "Lecture" << job;
        if (!_server.xmlTool().readJobFile(job)) {
            qDebug() << "Unable to read job " << job;
            continue;
        }
    }

    // construction arbre
    QStringList jobsNames = _server.xmlTool().getJobsNames();
    foreach(QString jobName, jobsNames) {
        JobDefinition * jobDef = _server.xmlTool().getJob(jobName);

        if (jobDef) {
            addJobInTree(jobDef);
        }
    }

    _ui->_TRW_assemblies->sortItems(0, Qt::AscendingOrder);
    _ui->_TRW_assemblies->expandAll();
}

bool AssemblyGui::getAssemblyValues(QString fileWithPath, QString &name, bool &valid, KeyValueList &assemblyValues)
{
    QFile assemblyFile(fileWithPath);
    QTextStream is(&assemblyFile);
    if (!assemblyFile.open(QIODevice::ReadOnly)) {
        qDebug() << "Erreur ouverture lecture" << fileWithPath;
        return false;
    }

    // recherche des valeurs
    int processorCount = 0;
    int posStart =-1;
    int posEnd = -1;
    QString eltName = "";
    while (!is.atEnd()) {
        QString line = is.readLine().trimmed();
        if  (line.startsWith("<MatisseAssembly ")) {
            posStart = line.indexOf("name=\"") + 6;
            posEnd = line.indexOf("\"", posStart);
            name = line.mid(posStart, posEnd - posStart).trimmed();
            if (name.isEmpty()) {
                assemblyFile.close();
                return false;
            }
            posStart = line.indexOf("valid=\"") + 7;
            posEnd = line.indexOf("\"", posStart);
            valid = line.mid(posStart, posEnd - posStart).trimmed().toShort();
            qDebug() << "Valid=" << valid;

        } else if (line == "<Version>") {
            line = is.readLine().trimmed();
            assemblyValues.set("Version", line);
            qDebug() << "Version=" << line;
        } else if (line == "<Date>") {
            line = is.readLine().trimmed();
            qDebug() << "Date=" << line;
            assemblyValues.set("Date", line);
        } else if (line == "<Author>") {
            line = is.readLine().trimmed();
            qDebug() << "Author=" << line;
            assemblyValues.set("Auteur", line);
        } else if (line == "<Comments>") {
            QString buffer;
            while ((line = is.readLine().trimmed()) != "</Comments>") {
                buffer.append(line + "\n");
            }
            buffer.chop(1);
            qDebug() << "Comments=" << buffer;
            assemblyValues.set("Commentaires", buffer);
        } else if (line.startsWith("<Parameters ")) {
            posStart = line.indexOf("name=\"") + 6;
            posEnd = line.indexOf("\"", posStart);
            eltName = line.mid(posStart, posEnd - posStart).trimmed();
            assemblyValues.set("Parametres", eltName);
        } else if (line.startsWith("<Source ")) {
            posStart = line.indexOf("name=\"") + 6;
            posEnd = line.indexOf("\"", posStart);
            eltName = line.mid(posStart, posEnd - posStart).trimmed();
            assemblyValues.set("Source", eltName);
        } else if (line.startsWith("<Processor ")) {
            processorCount++;
            posStart = line.indexOf("name=\"") + 6;
            posEnd = line.indexOf("\"", posStart);
            eltName = line.mid(posStart, posEnd - posStart).trimmed();
            assemblyValues.set(QString("Processeur n°%1").arg(processorCount), eltName);
        } else if (line == "</Processors>") {
            break;
        }
    }
    assemblyFile.close();

    return true;
}

void AssemblyGui::slot_selectAssemblyOrJob(QTreeWidgetItem * selectedItem, int column)
{
    QString name = selectedItem->text(column);
    _ui->_TRW_assemblyInfo->clear();
    if(_userFormWidget) {
        _userFormWidget->clear();
    }

    setActionsStates(selectedItem);

    if (!selectedItem->parent()) {
        // On a selectionné un assemblage

        QString filename = selectedItem->data(column, Qt::UserRole).toString();
        if (_server.xmlTool().readAssemblyFile(filename)) {

            AssemblyDefinition * selectedAssembly = _server.xmlTool().getAssembly(name);
            if (selectedAssembly) {
                QString parametersVersion = selectedAssembly->parametersDefinition()->model();
                QString modelFile = _rootXml + QDir::separator() + "models" + QDir::separator() + "parameters" + QDir::separator() + "Parameters_" + parametersVersion + ".xml";
                QString parametersFile = _rootXml + QDir::separator() + "parameters" + QDir::separator() + parametersVersion + QDir::separator() + selectedAssembly->parametersDefinition()->name().replace(" ", "_") + ".xml";
                //                if (!_parameters) {
                //                    _parameters = new Tools();
                //                }
                if (_parameters->readUserParametersFile(parametersFile, modelFile)) {
                    _userFormWidget->showUserParameters(_parameters);
                }
                // chargement des infos

                new QTreeWidgetItem(_ui->_TRW_assemblyInfo, QStringList() << "Version:" << selectedAssembly->version());
                new QTreeWidgetItem(_ui->_TRW_assemblyInfo, QStringList() << "Date de création:" << selectedAssembly->date());
                new QTreeWidgetItem(_ui->_TRW_assemblyInfo, QStringList() << "Auteur:" << selectedAssembly->author());
                new QTreeWidgetItem(_ui->_TRW_assemblyInfo, QStringList() << "Commentaire:" << selectedAssembly->comment());

                if (!_userMode) {
                    _expertFormWidget -> loadAssembly(name);
                    _expertFormWidget->showParameters(selectedAssembly);
                }
            } else {
                _userFormWidget->showUserParameters(NULL);
            }
        }


    } else {
        JobDefinition * selectedJob = _server.xmlTool().getJob(name);
        if (selectedJob) {
            _currentJobName = selectedJob->name();
            // chargement des parametres
            // le modele est dans (rootXml)/models/parameters
            // les parametres utilisateurs est dans (rootXml)/users/parameters
            //            if (!_parameters) {
            //                _parameters = new Tools();
            //            }

            QString parametersVersion = selectedJob->parametersDefinition()->model();
            QString modelFile = _rootXml + QDir::separator() + "models" + QDir::separator() + "parameters" + QDir::separator() + "Parameters_" + parametersVersion + ".xml";
            QString parametersFile = _rootXml + QDir::separator() + "users" + QDir::separator() + "parameters" + QDir::separator() + parametersVersion + QDir::separator() + selectedJob->parametersDefinition()->name().replace(" ", "_") + ".xml";
            if (_parameters->readUserParametersFile(parametersFile, modelFile)) {
                _userFormWidget->showUserParameters(_parameters);
            }

            // chargement des infos
            QString comments = selectedJob->comment();

            new QTreeWidgetItem(_ui->_TRW_assemblyInfo, QStringList() << "Commentaire:" << comments);

            if (selectedJob->executionDefinition()) {
                if (selectedJob->executionDefinition()->executed()) {
                    new QTreeWidgetItem(_ui->_TRW_assemblyInfo, QStringList() << "Date d'exécution:" << selectedJob->executionDefinition()->executionDate().toString("le dd/MM/yyyy à HH:mm"));
                    new QTreeWidgetItem(_ui->_TRW_assemblyInfo, QStringList() << "Image résultat:" << selectedJob->executionDefinition()->resultFileName());
                }
            }
            // si le job a été exécuté, chargement de l'image
            QString resultFile = selectedJob->executionDefinition()->resultFileName();
            if (selectedJob->executionDefinition()->executed() && (!resultFile.isEmpty())) {
                // affichage de l'image
                QFileInfo infoImage(resultFile);
                if (infoImage.isRelative()) {
                    infoImage.setFile(QDir(_dataPath), resultFile);
                }
                if (!infoImage.exists()) {
                    qDebug() << "Erreur fichier image introuvable" << infoImage.absoluteFilePath();
                }else{
                    qDebug() << "Chargement de l'image :" << infoImage.absoluteFilePath();
                    _userFormWidget->loadRasterFile(infoImage.absoluteFilePath());
                }
            }
        }
        else {
            _currentJobName = "";
        }
    }

}

void AssemblyGui::slot_assemblyElementsCount(int count)
{
    // test du contenu de l'assemblage pour l'etat des boutons save et save as...
    // on est en mode expert...
    // A unifier avec l'etat des boutons...
//    bool elements = (count > 0);
//    bool noJobs = false;
//    if (_ui->_TRW_assemblies->currentItem()) {
//        noJobs =(_ui->_TRW_assemblies->currentItem()->childCount() == 0);
//    }
//    _ui->_ACT_deleteAssembly->setEnabled(elements && noJobs);
//    _ui->_ACT_saveAssembly->setEnabled(elements && noJobs);
//    _ui->_ACT_saveAsAssembly->setEnabled(elements);
}

void AssemblyGui::slot_saveParameters()
{
    if (_userMode) {
        return;
    }

    if (_expertFormWidget->saveParameters()) {
        // on recharge les
        _expertFormWidget->selectLastUsedParameter();
        _expertValuesModified = false;
        _ui->_ACT_saveAssembly->setEnabled(true);
        _ui->_ACT_saveAsAssembly->setEnabled(true);
    }
}

void AssemblyGui::slot_showAssembly(QModelIndex index)
{
    if (_userFormWidget->isVisible()) {
        return;
    }
    if (index.parent().isValid()) {
        return;
    }
    QString filename =  index.sibling(index.row(), 0).data(Qt::UserRole).toString();
    qDebug() << "Show assembly:" << filename;
    _expertFormWidget -> loadAssembly(filename);

}

void AssemblyGui::slot_saveAssembly()
{
    // au cas où....
    if (!_userMode) {
        qDebug() << "Save assembly";
        QString name = _ui->_TRW_assemblies->currentItem()->text(0);
        KeyValueList fields;

        //        AssemblyDialog dialog(this, name, fields);
        //        if (dialog.exec() != QDialog::Accepted) {
        //            return;
        //        }

        // test modification parameteres...
        //if ()
        _expertFormWidget -> saveAssembly(name, fields);
    }
}

void AssemblyGui::slot_saveAsAssembly()
{
    // au cas où....
    if (!_userMode) {
        qDebug() << "Save as assembly";
        QString name;
        KeyValueList fields;

        AssemblyDialog dialog(this, name, fields);
        if (dialog.exec() != QDialog::Accepted) {
            return;
        }

        _expertFormWidget -> saveAssembly(name, fields);
        // ajout dans la liste des assemblages
        // a modifier...
        // on recharge
        slot_assembliesReload();
        // puis on selectionne...
        selectAssemblyInTree(name);

    }
}


void AssemblyGui::selectAssemblyInTree(QString assemblyName) {
    for (int index=0; index < _ui->_TRW_assemblies->topLevelItemCount(); index++) {
        QTreeWidgetItem * item = _ui->_TRW_assemblies->topLevelItem(index);
        if (item->text(0) == assemblyName) {
            _ui->_TRW_assemblies->setCurrentItem(item);
            slot_selectAssemblyOrJob(item);
            break;
        }
    }
}

void AssemblyGui::slot_deleteAssembly()
{
    // au cas où bis...
    if (_userMode) {
        return;
    }
    qDebug() << "Delete assembly";
    QTreeWidgetItem * selectedItem = _ui->_TRW_assemblies->currentItem();
    if (!selectedItem) {
        // rien de sélectionné...
        return;
    }
    if (selectedItem->childCount() > 0) {
        // des jobs existent... impossible de supprimer
        // traité ailleurs normalement...
        return;
    }

    QString assemblyName = selectedItem->text(0);
    QString filename = _server.xmlTool().getAssembly(assemblyName)->filename();
    // suppression fichier assemblage
    filename = _server.xmlTool().getAssembliesPath() + QDir::separator() + filename;
    QFile file(filename);
    if (!file.exists()) {
        // erreur...
        return;
    }

    if (!file.remove()) {
        // erreur
        return;
    }
    // message effacement OK
    // suppression liste: rechargement...
    slot_assembliesReload();

}

void AssemblyGui::slot_saveJob()
{
    // au cas où...one more time...
    if (!_userMode) {
        return;
    }

    QTreeWidgetItem * item = _ui->_TRW_assemblies->currentItem();
    if (!item) {
        return;
    }
    qDebug() << "Save job";
    QString jobName = item->text(0);
    JobDefinition * job = _server.xmlTool().getJob(jobName);

    if (!job) {
        return;
    }

    // modif logique: on peut enregistrer un job deja executé: on enleve l'etat executed et on ecrase...
    if (job->executionDefinition()->executed()) {
        if (QMessageBox::No == QMessageBox::question(this, "Travail exécuté...",
                                                     "Le travail a déjà été exécuté.\nVoulez-vous l'écraser?",
                                                     QMessageBox::Yes,
                                                     QMessageBox::No)) {
            return;
        }
        job->executionDefinition()->setExecuted(false);
        job->executionDefinition()->setExecutionDate(QDateTime());
        job->executionDefinition()->setResultFileName("");
        // changement etat led + effacement image
        item->setIcon(0, QIcon(":/png/led-grey.png"));
        _userFormWidget->clear();
    }


    if (!_server.xmlTool().writeJobFile(job, true)) {
        showError("Fichier de travail...", "Le fichier " + job->name() + ".xml\nn'a pu être écrit...");
        return;
    }
    // enregistrement des parameters
    KeyValueList comments;
    QString parametersVersion = job->parametersDefinition()->model();
    QStringList pathParts(QStringList() << _server.xmlTool().getBasePath() << "users" << "parameters" << parametersVersion << job->name() + ".xml");
    _parameters->generateParametersFile(pathParts.join(QDir::separator()), comments);
    // on rechargement lees parameteres afin de valider l'ecriture...
    slot_selectAssemblyOrJob(item);
}

void AssemblyGui::slot_saveAsJob()
{
    // au cas où...one more time...
    if (!_userMode) {
        return;
    }

    QTreeWidgetItem * item = _ui->_TRW_assemblies->currentItem();
    if (!item) {
        return;
    }
    qDebug() << "Save as job";
    KeyValueList keysValues;
    keysValues.append("name");
    keysValues.append("comment");
    JobDialog dialog(this, &keysValues, _server.xmlTool().getJobsPath());
    if (dialog.exec() != QDialog::Accepted) {
        return;
    }

    QString assemblyName = item->text(0);
    if (item->parent() != NULL){
        assemblyName = item->parent()->text(0);
    }


    AssemblyDefinition * assembly = _server.xmlTool().getAssembly(assemblyName);
    if (!assembly) {
        if (!_server.xmlTool().readAssemblyFile(assemblyName + ".xml")) {
            showError("Fichier d'assemblage...", "Le fichier " + assemblyName + ".xml\nn'est pas valide ou est illisible...");
            return;
        }
    }
    QString assemblyVersion = assembly->version().simplified();
    QString parametersVersion = assembly->parametersDefinition()->model();

    JobDefinition newJob(keysValues.getValue("name"), assemblyName, assemblyVersion);
    newJob.setComment(keysValues.getValue("comment"));

    ParameterDefinition parameters(parametersVersion, newJob.name());
    newJob.setParametersDefinition(&parameters);
    if (!_server.xmlTool().writeJobFile(&newJob)) {
        showError("Fichier de travail...", "Le fichier " + newJob.name() + ".xml\nn'a pu être écrit...");
        return;
    }
    // enregistrement des parameters
    KeyValueList comments;

    QStringList pathParts(QStringList() << _server.xmlTool().getBasePath() << "users" << "parameters" << parametersVersion << newJob.name() + ".xml");
    _parameters->generateParametersFile(pathParts.join(QDir::separator()), comments);
    // ajout dans la liste: rechargement
    slot_assembliesReload();
    // reselection...on parcourt l'arbre
    for (int indexAssembly = 0; indexAssembly < _ui->_TRW_assemblies->topLevelItemCount(); indexAssembly++) {
        QTreeWidgetItem * assemblyItem = _ui->_TRW_assemblies->topLevelItem(indexAssembly);
        if (assemblyItem->text(0) == assemblyName) {
            for (int indexJob = 0; indexJob < assemblyItem->childCount(); indexJob++) {
                QTreeWidgetItem * jobItem = assemblyItem->child(indexJob);
                if (jobItem->text(0) == newJob.name()) {
                    // selection de l'item...
                    _ui->_TRW_assemblies->setCurrentItem(jobItem);
                    slot_selectAssemblyOrJob(jobItem);
                    break;
                }
            }
            break;
        }
    }
        //    QTreeWidgetItem * jobItem = new QTreeWidgetItem(_ui->_TRW_assemblies->currentItem(), QStringList() << keysValues.getValue("name"));
//    jobItem->setData(0, Qt::UserRole, assemblyName);
//    jobItem->setIcon(0, QIcon(":/png/led-grey.png"));
//    _ui->_TRW_assemblies->sortItems(0, Qt::AscendingOrder);
//    _ui->_TRW_assemblies->setCurrentItem(jobItem);

}

void AssemblyGui::slot_deleteJob()
{
    // au cas où bis...
    if (!_userMode) {
        return;
    }
    qDebug() << "Delete job";
    QTreeWidgetItem * selectedItem = _ui->_TRW_assemblies->currentItem();
    if (!selectedItem) {
        // rien de sélectionné...
        return;
    }
    if (selectedItem->parent() == NULL) {
        // assemblage... impossible de supprimer
        // traité ailleurs normalement...
        return;
    }

    QString jobName = selectedItem->text(0);
    QString filename = _server.xmlTool().getJob(jobName)->filename();
    // suppression fichier job
    // pour l'instant on n'efface pas le fichier de paramètres...
    // TODO: unicité utilisation patramètres...
    filename = _server.xmlTool().getJobsPath() + QDir::separator() + filename;
    QFile file(filename);
    if (!file.exists()) {
        // erreur...
        return;
    }

    if (!file.remove()) {
        // erreur
        return;
    }
    // message effacement OK
    // suppression liste: rechargement...
    slot_assembliesReload();

}

void AssemblyGui::showError(QString title, QString message) {
    QMessageBox::warning(this, title, message);
}

QTreeWidgetItem *AssemblyGui::addAssemblyInTree(AssemblyDefinition * assembly)
{
    QTreeWidgetItem * assemblyItem = new QTreeWidgetItem(QStringList() << assembly->name());
    assemblyItem->setData(0, Qt::UserRole,assembly->filename());
    _ui->_TRW_assemblies->addTopLevelItem(assemblyItem);
    _assembliesItems.insert( assembly->name(), assemblyItem);

    return assemblyItem;
}

QTreeWidgetItem *AssemblyGui::addJobInTree(JobDefinition * job)
{
    QString assembly = job->assemblyName();
    if (job->executionDefinition() == NULL) {
        qDebug() << "TOTO";
    }
    bool executed = job->executionDefinition()->executed();
    QTreeWidgetItem * assemblyItem = _assembliesItems.value(assembly, NULL);
    if (!assemblyItem) {
        // L'assemblage n'existe pas...
        return NULL;
    }
    QTreeWidgetItem * jobItem = new QTreeWidgetItem(assemblyItem, QStringList() << job->name());
    jobItem->setData(0, Qt::UserRole, assembly);
    if (executed) {
        jobItem->setIcon(0, QIcon(":/png/led-green.png"));
    } else {
        jobItem->setIcon(0, QIcon(":/png/led-grey.png"));
    }

    return jobItem;
}

void AssemblyGui::initStatusBar()
{
//    _statusProgressBar.setRange(0, 1);
//    _statusProgressBar.setValue(0);
    _statusLed.setScaledContents(true);
    _statusLed.setMinimumSize(32, 32);
    _statusLed.setMaximumSize(32, 32);

    _messagesIndicators.insert(IDLE, ":/png/led-grey.png");
    _messagesIndicators.insert(OK, ":/png/led-green.png");
    _messagesIndicators.insert(WARNING, ":/png/led_orange.png");
    _messagesIndicators.insert(ERROR, ":/png/led_red.png");

    // ProgressBar pour etat... a ajouter
    //statusBar()->addWidget(&_statusProgressBar);
//    int comboWidth = qMin(width()-40, width() * 8 / 10);
//    _messagesCombo.setMinimumWidth(comboWidth);
//    _messagesCombo.setMinimumWidth(4*_ui->_TRW_assemblies->width());
    //_messagesCombo.setSizePolicy(QSizePolicy::MinimumExpanding, QSizePolicy::Fixed);
//    _messagesResetButton.setIcon(QIcon(":/png/edit-delete-3.png"));
//    _messagesResetButton.setIconSize(QSize(20,20));
    statusBar()->addWidget(&_statusMessageWidget);
    //statusBar()->addWidget(&_messagesResetButton);
    statusBar()->addPermanentWidget(&_statusLed);

    showStatusMessage();
}

void AssemblyGui::showStatusMessage(QString message, MessageIndicatorLevel level, bool progressOn)
{
    if (message.isEmpty()) {
        return;
    }
    _statusMessageWidget.addMessage(message, QIcon(_messagesIndicators.value(level)));
   // _messagesCombo.insertItem(0, QIcon(_messagesIndicators.value(level)), message);
   // _messagesCombo.setCurrentIndex(0);
//    statusBar()->showMessage(message);
//    if (progressOn) {
//        statusBar()->insertWidget(0, &_statusProgressBar);
//    } else {
//        statusBar()->removeWidget(&_statusProgressBar);
//    }
    _statusLed.setPixmap(QPixmap(_messagesIndicators.value(level)));
}

void AssemblyGui::setActionsStates(QTreeWidgetItem *currentItem/*, bool modifiedConfiguration*/)
{
    bool saveStatus = false;
    bool saveAsStatus = false;
    bool deleteStatus = false;
    bool parametersStatus = false;
    bool launchStatus = false;

    //on disable tout pour commencer...
    _ui->_ACT_newAssembly->setEnabled(false);
    _ui->_ACT_saveAssembly->setEnabled(false);
    _ui->_ACT_saveAsAssembly->setEnabled(false);
    _ui->_ACT_deleteAssembly->setEnabled(false);
    _ui->_ACT_saveParameters->setEnabled(false);
    _ui->_ACT_saveJob->setEnabled(false);
    _ui->_ACT_saveAsJob->setEnabled(false);
    _ui->_ACT_deleteJob->setEnabled(false);
    _ui->_ACT_launchJob->setEnabled(false);

    if (currentItem) {
        if (_userMode) {
            if (currentItem->parent()) {
                // on a affaire à un job...
                launchStatus = true;
                QString jobName = currentItem->text(0);
                JobDefinition * job = _server.xmlTool().getJob(jobName);
                if (!job->executionDefinition()->executed()) {
                    saveStatus = true;
                    saveAsStatus = true;
                    deleteStatus = true;
                } else {
                    // modif: dans tous les cas on peut ecraser ou supprimer
                    saveStatus = true;
                    saveAsStatus = true;
                    deleteStatus = true;
                }
            } else {
                // on a affaire à un assemblage
                // on ne peut que creer un nouveau job
                saveAsStatus = true;
            }
            _ui->_ACT_saveJob->setEnabled(saveStatus);
            _ui->_ACT_saveAsJob->setEnabled(saveAsStatus);
            _ui->_ACT_deleteJob->setEnabled(deleteStatus);
            _ui->_ACT_launchJob->setEnabled(launchStatus);

        } else {
            // mode expert
            saveAsStatus = true;
            parametersStatus = true;
            if (currentItem->childCount() == 0) {
                saveStatus = true;
                deleteStatus = true;
            }

            _ui->_ACT_newAssembly->setEnabled(true);
            _ui->_ACT_saveAssembly->setEnabled(saveStatus);
            _ui->_ACT_saveAsAssembly->setEnabled(saveAsStatus);
            _ui->_ACT_deleteAssembly->setEnabled(deleteStatus);
            _ui->_ACT_saveParameters->setEnabled(parametersStatus);
        }
    } else {
        // rustine...à modifier...
        if (!_userMode) {
            _ui->_ACT_saveParameters->setEnabled(true);
         // nouvel assemblage...
            _ui->_ACT_newAssembly->setEnabled(true);
            _ui->_ACT_saveAsAssembly->setEnabled(true);
        }
    }
}

void AssemblyGui::slot_quit()
{
    close();
}

void AssemblyGui::slot_clearAssembly()
{
    _expertFormWidget -> resetAssemblyForm();
    setActionsStates();
}

void AssemblyGui::slot_swapUserOrExpert()
{
    qDebug() << "Change user/expert=" << _ui->_ACT_userExpert->isChecked();

    if (_ui->_ACT_userExpert->isChecked()) {
        qDebug() << "Mode expert";
        _userMode = false;
        _userFormWidget->hide();
        _expertFormWidget->show();
        _ui->_TRW_assemblies->collapseAll();
        _ui->_TRW_assemblies->setItemsExpandable(false);
        slot_clearAssembly();
    } else {
        if (_expertValuesModified) {
            if (QMessageBox::No == QMessageBox::question(this, "Assemblages/paramètres modifiés...", "Voulez-vous continuer sans sauvegarder?",
                                      QMessageBox::Yes,
                                      QMessageBox::No)) {
                _ui->_ACT_userExpert->setChecked(true);
                return;
            }
            _expertValuesModified = false;
        }
        qDebug() << "Mode user";
        _userMode = true;
        _expertFormWidget->hide();
        _userFormWidget->show();
        // Changement des info bulles
        _ui->_TRW_assemblies->setItemsExpandable(true);
        _ui->_TRW_assemblies->expandAll();
        _userFormWidget->resetJobForm();
    }
    _ui->_TRW_assemblies->clearSelection();
    // TODO: autres actions....
}

void AssemblyGui::slot_launchJob()
{
    // On test assemblage ou job
    // Le bouton est actif si le job est selectionnable
    // cad si l'image n'a pas été produite...
    // ou si on a selectionné un assemblage
    // dans ce cas, on sauvegarde le job avant de le lancer...

    QTreeWidgetItem * currentItem = _ui->_TRW_assemblies->currentItem();
    QTreeWidgetItem * parentItem = currentItem->parent();

    QString assemblyName = currentItem -> text(0);
    QString jobName = assemblyName;
    KeyValueList jobDesc;

    // test modifs...
    bool saveAs = false;
    if (_userFormWidget->parametersWidget()->hasModifiedValues()) {
        saveAs = true;
    }

    if (parentItem) {
        // on a directement un job
        assemblyName = parentItem -> text(0);
        // Modif: si le job a été lancé, on peut le reexecuter avec warning...
        JobDefinition * job = _server.xmlTool().getJob(jobName);
        if (job->executionDefinition()->executed()) {
            if (QMessageBox::No == QMessageBox::question(this, "Travail exécuté...",
                                                         "Le travail a déjà été exécuté.\nVoulez-vous le relancer?",
                                                         QMessageBox::Yes,
                                                         QMessageBox::No)) {
                return;
            }
        }

        // si un parametre est modifié, on demande si on sauvegarde sous un nouveau nom...
        // INFO: normalement on ne doit plus passer ici suite à la modification de logique des enregistrements...
        if (saveAs) {
            if (QMessageBox::No == QMessageBox::question(this, "Modification de paramètres...",
                                                         "Un ou plusieurs paramètres ont été modifiés.\nVoulez-vous enregistrer le travail sous un autre nom?",
                                                         QMessageBox::Yes,
                                                         QMessageBox::No)) {
                saveAs = false;
            }
        }
    }
    // on crée un nouveau job à partir d'un assemblage vide: sauvegarde obligatoire:
    // INFO: saveAs n'a plus lieu d'être ici suite modif de la logique des enregistrements...
    if ((!parentItem) || saveAs){
        // on a un assemblage
        // on crée le job...
        // ou on sauvegarde l'assemblage sous un autre nom...
        jobName = JobDialog::newJobName(this, &jobDesc, _server.xmlTool().getJobsPath());
        if (jobName.isEmpty()) {
            showStatusMessage("Travail annulé...");
            // annulé...
            return;
        }
    }

    AssemblyDefinition * assemblyDef = _server.xmlTool().getAssembly(assemblyName);
    if (!assemblyDef) {
        qDebug() << "Erreur recup assemblage" << assemblyName;
        showStatusMessage("Erreur sur l'assemblage", ERROR);
        return;
    }

    JobDefinition * job = _server.xmlTool().getJob(jobName);
    QString jobFilename;

    if (!job) {
        // ecriture du fichier de parametres
        QStringList args= QStringList() << _server.xmlTool().getBasePath()
                                        << "users" << "parameters"
                                        << assemblyDef->parametersDefinition()->model()
                                        << jobDesc.getValue("filename");

        _parameters->generateParametersFile(args.join(QDir::separator()));

        // nouveau job
        job = new JobDefinition(jobName, assemblyName, assemblyDef->version());
        ParameterDefinition * newParam = new ParameterDefinition(assemblyDef->parametersDefinition()->model(), jobName);
        job->setParametersDefinition(newParam);
        jobFilename = jobDesc.getValue("filename");
        job->setComment(jobDesc.getValue("comment"));
        job->setFilename(jobFilename);
        job->setExecutionDefinition(new ExecutionDefinition());
        if (!_server.xmlTool().writeJobFile(job, true)) {
            // erreur ecriture
            QMessageBox::information(this, "Erreur d'écriture", "Le fichier de travail " + jobName + " n'a pu être écrit");
            showStatusMessage("Erreur d'écriture. Le fichier de travail " + jobName + " n'a pu être écrit", ERROR);
            return;
        }
        // ajout du job dans la liste et selection
        currentItem = addJobInTree(job);
        if (currentItem) {
            _ui->_TRW_assemblies->sortItems(0, Qt::AscendingOrder);
            currentItem->setSelected(true);
        }


    } else {
        jobFilename = job->filename();
    }

    qDebug() << "Execute le job " << jobName;
    // on recharge le fichier...
    if (!jobName.isEmpty()) {
        if (_server.xmlTool().readJobFile(jobFilename)) {
            JobDefinition *job = _server.xmlTool().getJob(jobName);

            if (job) {
                _lastJobLaunchedItem = currentItem;
                showStatusMessage("Travail " + jobName + " en cours...", IDLE, true);
                if (!_server.processJob(*job)) {
                    showStatusMessage("Erreur " + jobName + ": " + _server.messageStr(), ERROR, true);
                }
            } else {
                QMessageBox::information(this, "Fichier introuvale", "Le fichier de travail " + jobName + " n'a pu être lancé");
                showStatusMessage("Le fichier de travail " + jobName + " n'a pu être lancé", ERROR);

            }
        }
    }

}

void AssemblyGui::slot_jobProcessed(QString name) {
    qDebug() << "Job done : " << name;
    if (!_server.errorFlag()) {
        JobDefinition *jobDef = _server.xmlTool().getJob(name);
        jobDef->executionDefinition()->setExecuted(true);
        QDateTime now = QDateTime::currentDateTime();
        jobDef->executionDefinition()->setExecutionDate(now);
        _server.xmlTool().writeJobFile(jobDef, true);
        if (_lastJobLaunchedItem) {
            // mise a jour de la led...
            _lastJobLaunchedItem->setIcon(0, QIcon(":/png/led-green.png"));
            showStatusMessage("Travail " + jobDef->name() + " terminé...", OK);
        }
    }
}

void AssemblyGui::slot_assembliesReload()
{
    loadAssembliesAndJobsLists();

    if (_userMode) {
        _userFormWidget->clear();
        qDebug() << "Clear userForm...";
    } else {
        slot_clearAssembly();
    }

    slot_swapUserOrExpert();

}

void AssemblyGui::slot_modifiedParameters(bool changed)
{
    QObject * sendObjet = sender();
    if (sendObjet == _userFormWidget) {
        // on disable le bouton de lancement si true et si ce n'est pas un assemblage qui est sélectionné...
        QTreeWidgetItem * currentItem = _ui->_TRW_assemblies->currentItem();
        if (currentItem && (currentItem->parent() == NULL)) {
            changed = true;
        }
        _ui->_ACT_launchJob->setDisabled(changed);
        qDebug() << "Disable bouton slot_modifiedParameters" << changed;
        _userParameterModified = changed;
    } else {
        qDebug() << "expert parameters modified";
        _expertValuesModified = changed;
        // on disable les boutons de sauvegarde pour forcer l'enregistrement ou le rechargement
        // Commenté car NOK aaprès un new assembly...
        //        _ui->_ACT_saveAssembly->setDisabled(true);
        //        _ui->_ACT_saveAsAssembly->setDisabled(true);
    }
}

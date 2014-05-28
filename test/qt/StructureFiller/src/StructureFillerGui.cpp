#include "StructureFillerGui.h"
#include "ui_StructureFillerGui.h"

StructureFillerGui::StructureFillerGui(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::StructureFillerGui),
    _wrapper(0)
{
    ui->setupUi(this);
    init();
}

StructureFillerGui::~StructureFillerGui()
{
    if (_currentWidget) {
        _currentWidget->deleteLater();
    }

    delete ui;
}

void StructureFillerGui::init()
{
    _currentWidget = ui->_WID_dummy;
    //_wrapper.readParametersFile("datas/Datas.xml");
    connect(ui->_LW_structures, SIGNAL(clicked(QModelIndex)), this, SLOT(slot_selectStructure(QModelIndex)));
    connect(ui->_ACT_openModelXML, SIGNAL(triggered()), this, SLOT(slot_openXML()));
    connect(ui->_ACT_openParametersXML, SIGNAL(triggered()), this, SLOT(slot_openParametersFile()));
    connect(ui->_ACT_saveParametersXML, SIGNAL(triggered()), this, SLOT(slot_saveParametersFile()));
    connect(ui->_ACT_quit, SIGNAL(triggered()), this, SLOT(slot_quit()));
    connect(ui->_RB_user, SIGNAL(clicked(bool)), this, SLOT(slot_showUserParams(bool)));
    connect(ui->_RB_expert, SIGNAL(clicked(bool)), this, SLOT(slot_showUserParams(bool)));

    ui->_ACT_openParametersXML->setEnabled(false);
    ui->_ACT_saveParametersXML->setEnabled(false);

    ui->_RB_user->setEnabled(false);
    ui->_RB_expert->setEnabled(false);


    // création des répertoires par défaut: datas et generated si inexistant
    QDir dir = QDir::current();
//    if (!dir.exists("../MatisseData/xml")) {
//        dir.mkdir("datas");
//    }
    if (!dir.exists("generated")) {
        dir.mkdir("generated");
    }
}

void StructureFillerGui::slot_selectStructure(QModelIndex index)
{
    if (!index.isValid()) {
        return;
    }
    ui->_RB_user->setEnabled(true);
    ui->_RB_expert->setEnabled(true);
    ui->_RB_expert->setChecked(true);

    _currentStructName = index.data().toString();
    ui->_GB_parameters->layout()->removeWidget(_currentWidget);
    _currentWidget->setVisible(false);
    _currentWidget = _wrapper->createDialog(_currentStructName);
    ui->_GB_parameters->layout()->addWidget(_currentWidget);
    _currentWidget->setVisible(true);
}

void StructureFillerGui::slot_quit()
{
    close();
}

void StructureFillerGui::slot_openXML()
{
    QString filename = QFileDialog::getOpenFileName(this, "Fichiers descripteur de structures", ".", "*.xml");
    ui->_RB_user->setEnabled(false);
    ui->_RB_expert->setEnabled(false);
    if (filename != "") {
        if (_wrapper) {
            _wrapper->deleteLater();
        }
        _wrapper = new MatisseTools();
        if (!_wrapper->readParametersFile(filename)) {
            QMessageBox::warning(this, "Erreur de fichier", "Le fichier n'est pas au format attendu");
            ui->_LW_structures->clear();
            ui->_ACT_saveParametersXML->setEnabled(false);
            _currentWidget = ui->_WID_dummy;

            return;
        }

        ui->_LW_structures->clear();
        foreach(QString structname, _wrapper->structureNames()) {
            QListWidgetItem * item = new QListWidgetItem(structname, ui->_LW_structures);
            //TODO Pour test check sur item...
            //item->setFlags(item->flags() | Qt::ItemIsUserCheckable);
            //item->setCheckState(Qt::Checked);
        }
        _currentWidget = ui->_WID_dummy;

        ui->_ACT_saveParametersXML->setEnabled(true);
    }
}

void StructureFillerGui::slot_openParametersFile()
{
}

void StructureFillerGui::slot_saveParametersFile()
{
    QString filename = QFileDialog::getSaveFileName(this, "Fichier de paramètres Mosaic", "./generated", "*.xml");
    if (filename != "") {
        if (!_wrapper) {
            return;
        }
        _wrapper->generateInitFile(filename);
    }
}

void StructureFillerGui::slot_showUserParams(bool user)
{
    if (sender() == ui->_RB_expert) {
        user = false;
    }
    qDebug() << "Show user=" << user;
        ui->_GB_parameters->layout()->removeWidget(_currentWidget);
        _currentWidget->setVisible(false);
        _currentWidget = _wrapper->createDialog(_currentStructName, user);
        ui->_GB_parameters->layout()->addWidget(_currentWidget);
        _currentWidget->setVisible(true);
}

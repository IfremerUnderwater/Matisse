#include "assembly_editor.h"
#include "ui_assembly_editor.h"


using namespace matisse;
using namespace MatisseServer;


AssemblyEditor::AssemblyEditor(QWidget *parent) :
    QWidget(parent),
    _ui(new Ui::AssemblyEditor)//,
    //_server(NULL)
{
    _ui->setupUi(this);
    init();
}

AssemblyEditor::~AssemblyEditor()
{
    delete _ui;
}

void AssemblyEditor::init()
{
    _ui->_GRW_assembly->setEnabled(false);
    _ui->_GRW_assembly->setAcceptDrops(true);

    QRect containerRect = _ui->_GRW_assembly->rect();

    _scene = new AssemblyGraphicsScene(containerRect);
    _ui->_GRW_assembly->setScene(_scene);
    _ui->_GRW_assembly->centerOn(0, 0);
    _scene->initViewport();
}

void AssemblyEditor::resizeEvent(QResizeEvent *event)
{
    QWidget::resizeEvent(event);

    qDebug() << "ExpertFormWidget Resize event : " << event->oldSize() << event->size();
}

//void ExpertFormWidget::setServer(Server *server) {
//    _server = server;
//    _scene->setServer(server);
//}


QGraphicsView *AssemblyEditor::getGraphicsView()
{
    return _ui->_GRW_assembly;
}


bool AssemblyEditor::loadAssembly(QString assemblyName)
{
    qDebug() << "Load assembly:" << assemblyName;
    return _scene->loadAssembly(assemblyName);

}

void AssemblyEditor::resetAssemblyForm()
{
    _scene->reset();
    _ui->_GRW_assembly->invalidateScene();
    _ui->_GRW_assembly->update();
}

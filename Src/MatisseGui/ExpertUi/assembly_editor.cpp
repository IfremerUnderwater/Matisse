#include "assembly_editor.h"
#include "ui_assembly_editor.h"


namespace matisse {

AssemblyEditor::AssemblyEditor(QWidget *_parent) :
    QWidget(_parent),
    m_ui(new Ui::AssemblyEditor)
{
    m_ui->setupUi(this);
    init();
}

AssemblyEditor::~AssemblyEditor()
{
    delete m_ui;
}

void AssemblyEditor::init()
{
    m_ui->_GRW_assembly->setEnabled(false);
    m_ui->_GRW_assembly->setAcceptDrops(true);

    QRect container_rect = m_ui->_GRW_assembly->rect();

    m_scene = new AssemblyGraphicsScene(container_rect);
    m_ui->_GRW_assembly->setScene(m_scene);
    m_ui->_GRW_assembly->centerOn(0, 0);
    m_scene->initViewport();
}

void AssemblyEditor::resizeEvent(QResizeEvent *_event)
{
    QWidget::resizeEvent(_event);

    qDebug() << "ExpertFormWidget Resize event : " << _event->oldSize() << _event->size();
}

QGraphicsView *AssemblyEditor::getGraphicsView()
{
    return m_ui->_GRW_assembly;
}

bool AssemblyEditor::loadAssembly(QString _assembly_name)
{
    qDebug() << "Load assembly:" << _assembly_name;
    return m_scene->loadAssembly(_assembly_name);

}

void AssemblyEditor::resetAssemblyForm()
{
    m_scene->reset();
    m_ui->_GRW_assembly->invalidateScene();
    m_ui->_GRW_assembly->update();
}

} // namespace matisse

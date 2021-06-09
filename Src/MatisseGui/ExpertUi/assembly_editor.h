#ifndef MATISSE_ASSEMBLY_EDITOR_H_
#define MATISSE_ASSEMBLY_EDITOR_H_

#include <QWidget>
#include <QModelIndex>
#include <QListWidgetItem>
#include <QMessageBox>
#include <QScrollArea>
#include <QAbstractScrollArea>
#include <QResizeEvent>

#include "assembly_graphics_scene.h"

#include "source_widget.h"
#include "processor_widget.h"
#include "destination_widget.h"
#include "pipe_widget.h"
#include "assembly_graphics_scene.h"
#include "key_value_list.h"
#include "parameters_dialog.h"
#include "matisse_parameters_manager.h"
#include "parameters_widget_skeleton.h"


namespace Ui {
class AssemblyEditor;
}

namespace matisse {

class AssemblyEditor : public QWidget
{
    Q_OBJECT
    
public:
    explicit AssemblyEditor(QWidget *_parent = 0);
    ~AssemblyEditor();

    bool loadAssembly(QString _assembly_name);
    void resetAssemblyForm();
    AssemblyGraphicsScene * getScene() { return m_scene;}
    QGraphicsView * getGraphicsView();

private:
    Ui::AssemblyEditor * m_ui;
    AssemblyGraphicsScene * m_scene;
    QHash<QString, KeyValueList> m_assemblies_values;

    void init();

protected:
    void resizeEvent(QResizeEvent *_event);

protected slots:


signals:

};

} // namespace matisse

#endif // MATISSE_ASSEMBLY_EDITOR_H_

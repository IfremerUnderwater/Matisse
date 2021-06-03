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
    explicit AssemblyEditor(QWidget *parent = 0);
    ~AssemblyEditor();

    bool loadAssembly(QString assemblyName);
    void resetAssemblyForm();
    AssemblyGraphicsScene * getScene() { return _scene;}
    QGraphicsView * getGraphicsView();

private:
    Ui::AssemblyEditor * _ui;
    AssemblyGraphicsScene * _scene;
    QHash<QString, KeyValueList> _assembliesValues;
//    Server * _server;

    void init();

protected:
    void resizeEvent(QResizeEvent *event);

protected slots:
//    void slot_showParameters(QTreeWidgetItem *item = NULL, int noCol = 0);
    //void slot_parametersValuesModified(bool modified);
    //void slot_changeTabPanel(int panel);

signals:
    //void signal_parametersValuesModified(bool modified);
    //void signal_saveParameters();
    //void signal_selectParameters(bool);
    //void signal_usedParameters(bool);
};

} // namespace matisse

#endif // MATISSE_ASSEMBLY_EDITOR_H_

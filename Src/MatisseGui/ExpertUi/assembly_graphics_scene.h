#ifndef MATISSE_ASSEMBLY_GRAPHICS_SCENE_H_
#define MATISSE_ASSEMBLY_GRAPHICS_SCENE_H_

#include <QGraphicsScene>
#include <QGraphicsSceneMouseEvent>
#include <QGraphicsSceneDragDropEvent>
#include <QGraphicsLineItem>
#include <QPointF>
#include <QFile>
#include <QTextStream>
#include <QListWidget>
#include <QListWidgetItem>
#include <QtDebug>
#include <QTreeWidget>
#include <QTreeWidgetItem>
#include <QGraphicsView>
#include <QPointer>
#include <QMenu>
#include <QAction>
#include <QMainWindow>

#include "element_widget.h"
#include "element_widget_provider.h"
#include "source_widget.h"
#include "processor_widget.h"
#include "destination_widget.h"
#include "pipe_widget.h"
#include "key_value_list.h"
#include "matisse_engine.h"


namespace matisse {

class AssemblyGraphicsScene : public QGraphicsScene
{
    Q_OBJECT
public:
    explicit AssemblyGraphicsScene(const QRectF & _scene_rect, QObject * _parent = 0 );

    virtual void mousePressEvent(QGraphicsSceneMouseEvent *_event);
    virtual void mouseMoveEvent(QGraphicsSceneMouseEvent *_event);
    virtual void mouseReleaseEvent(QGraphicsSceneMouseEvent *_event);

    virtual void dragMoveEvent(QGraphicsSceneDragDropEvent *_event){_event->accept();}
    virtual void dropEvent(QGraphicsSceneDragDropEvent *_event);
    virtual bool event(QEvent *_event);
    void setEngine(MatisseEngine* _engine) { m_engine = _engine; }
    void setElementWidgetProvider(ElementWidgetProvider * _element_provider);
    void reset();

    void updateAssembly(AssemblyDefinition* _assembly);
    bool loadAssembly(QString _assembly_name);
    void setMessageTarget(QWidget *_target_widget);
    void initViewport();

private:
    QPointer<PipeWidget> m_pipe_item;
    QList<PipeWidget *> m_connectors;
    QWidget * m_viewport;
    QWidget * m_message_target_widget;

    ElementWidgetProvider * m_element_provider;
    MatisseEngine * m_engine;
    QPointer<SourceWidget> m_source_widget;
    QMap<quint8, ProcessorWidget *> m_processors_widgets;
    QPointer<DestinationWidget> m_destination_widget;
    static const quint16 ACTIVE_SCENE_WIDTH = 200;
    static const quint16 ACTIVE_SCENE_HEIGHT = 2400;
    static const quint16 INACTIVE_SCENE_WIDTH = 200;
    static const quint16 INACTIVE_SCENE_HEIGHT = 500;
    bool m_is_scene_active;
    bool m_is_assembly_modified;
    bool m_is_assembly_complete;

    void init();
    void checkAssemblyComplete();
    void applyAssemblyCompleteness(bool _is_complete);

signals:
    void si_itemsCount(int _count);
    void si_selectParameters(QString _parameters);
    void si_assemblyModified();
    void si_assemblyComplete(bool _is_complete, AssemblyDefinition *_assembly = NULL);

public slots:
    void sl_sceneChanged(const QList<QRectF> & _region);
};

} // namespace matisse

#endif // MATISSE_ASSEMBLY_GRAPHICS_SCENE_H_

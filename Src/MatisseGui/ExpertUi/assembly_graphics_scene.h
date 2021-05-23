#ifndef ASSEMBLYGRAPHICSSCENE_H
#define ASSEMBLYGRAPHICSSCENE_H

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

#include "ElementWidget.h"
#include "ElementWidgetProvider.h"
#include "SourceWidget.h"
#include "ProcessorWidget.h"
#include "DestinationWidget.h"
#include "PipeWidget.h"
#include "key_value_list.h"
#include "matisse_engine.h"


using namespace MatisseTools;

namespace MatisseServer {

class AssemblyGraphicsScene : public QGraphicsScene
{
    Q_OBJECT
public:
//    explicit AssemblyGraphicsScene(QObject *parent = 0);
    explicit AssemblyGraphicsScene(const QRectF & sceneRect, QObject * parent = 0 );

    virtual void mousePressEvent(QGraphicsSceneMouseEvent *event);
    virtual void mouseMoveEvent(QGraphicsSceneMouseEvent *event);
    virtual void mouseReleaseEvent(QGraphicsSceneMouseEvent *event);

    //virtual void dragEnterEvent(QGraphicsSceneDragDropEvent *event);
    virtual void dragMoveEvent(QGraphicsSceneDragDropEvent *event){event->accept();}
    virtual void dropEvent(QGraphicsSceneDragDropEvent *event);
    virtual bool event(QEvent *event);
    void setServer(MatisseEngine* server) { _server = server; }
    void setElementWidgetProvider(ElementWidgetProvider * elementProvider);
    void reset();

    //bool saveAssembly(QString filename, AssemblyDefinition *assembly);
    void updateAssembly(AssemblyDefinition* assembly);
    bool loadAssembly(QString assemblyName);
    void setMessageTarget(QWidget *targetWidget);
    void initViewport();

private:
    QPointer<PipeWidget> _pipeItem;
    QList<PipeWidget *> _connectors;
    QWidget * _viewport;
    QWidget * _messageTargetWidget;

    ElementWidgetProvider * _elementProvider;
    MatisseEngine * _server;
    QPointer<SourceWidget> _sourceWidget;
    QMap<quint8, ProcessorWidget *> _processorsWidgets;
    QPointer<DestinationWidget> _destinationWidget;
    static const quint16 ACTIVE_SCENE_WIDTH = 200;
    static const quint16 ACTIVE_SCENE_HEIGHT = 2400;
    static const quint16 INACTIVE_SCENE_WIDTH = 200;
    static const quint16 INACTIVE_SCENE_HEIGHT = 500;
    bool _isSceneActive;
    bool _isAssemblyModified;
    bool _isAssemblyComplete;

    void init();
    void checkAssemblyComplete();
    void applyAssemblyCompleteness(bool isComplete);

signals:
    void signal_itemsCount(int count);
    void signal_selectParameters(QString parameters);
    void signal_assemblyModified();
    void signal_assemblyComplete(bool isComplete, AssemblyDefinition *assembly = NULL);

public slots:
    void slot_sceneChanged(const QList<QRectF> & region);
};
}

#endif // ASSEMBLYGRAPHICSSCENE_H

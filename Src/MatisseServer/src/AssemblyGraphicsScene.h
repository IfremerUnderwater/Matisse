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

#include "ParametersWidget.h"
#include "ElementWidget.h"
#include "SourceWidget.h"
#include "ProcessorWidget.h"
#include "DestinationWidget.h"
#include "PipeWidget.h"
#include "KeyValueList.h"
#include "Server.h"

using namespace MatisseTools;

namespace MatisseServer {
class ExpertFormWidget;
class AssemblyGui;

class AssemblyGraphicsScene : public QGraphicsScene
{
    Q_OBJECT
public:
//    explicit AssemblyGraphicsScene(QObject *parent = 0);
    explicit AssemblyGraphicsScene(Server * server, const QRectF & sceneRect, QObject * parent = 0 );

    virtual void mousePressEvent(QGraphicsSceneMouseEvent *event);
    virtual void mouseMoveEvent(QGraphicsSceneMouseEvent *event);
    virtual void mouseReleaseEvent(QGraphicsSceneMouseEvent *event);

    virtual void dragEnterEvent(QGraphicsSceneDragDropEvent *event);
    virtual void dragMoveEvent(QGraphicsSceneDragDropEvent *event){event->accept();}
    virtual void dropEvent(QGraphicsSceneDragDropEvent *event);
    virtual bool event(QEvent *event);
    void setExpertGui(ExpertFormWidget * gui);
    void setMainGui(AssemblyGui * gui);
    void reset();

    bool saveAssembly(QString filename, AssemblyDefinition *assembly);
    bool loadAssembly(QString assemblyName);

private:
//    QPointF _startPos;
//    QPointF _endPos;
    QPointer<PipeWidget> _pipeItem;
    QList<PipeWidget *> _connectors;
    QWidget * _viewport;

    ExpertFormWidget * _expertGui;
    AssemblyGui * _mainGui;
    Server * _server;
    QPointer<ParametersWidget> _parametersWidget;
    QPointer<SourceWidget> _sourceWidget;
    QMap<quint8, ProcessorWidget *> _processorsWidgets;
    QPointer<DestinationWidget> _destinationWidget;

    void init();

signals:
    void signal_itemsCount(int count);
    void signal_selectParameters(QString parameters);

public slots:
    void slot_sceneChanged(const QList<QRectF> & region);
};
}

#endif // ASSEMBLYGRAPHICSSCENE_H

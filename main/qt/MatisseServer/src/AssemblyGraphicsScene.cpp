#include "AssemblyGraphicsScene.h"
#include "ExpertFormWidget.h"
#include "AssemblyGui.h"

using namespace MatisseServer;

AssemblyGraphicsScene::AssemblyGraphicsScene(Server * server, const QRectF &sceneRect, QObject *parent) :
    QGraphicsScene(sceneRect, parent),
    _server(server)
{
    //    _startPos = QPointF();
    //    _endPos = QPointF();
    init();

}

void AssemblyGraphicsScene::init() {
    setSceneRect(0, 0, 200, 1000);
    _pipeItem = new PipeWidget();
    //    _pipeItem->setVisible(false);
    //    addItem(_pipeItem);
    _sourceWidget = NULL;
    _parametersWidget = NULL;
    _destinationWidget = NULL;
    connect(this, SIGNAL(changed(QList<QRectF>)), this, SLOT(slot_sceneChanged(QList<QRectF>)));
}

void AssemblyGraphicsScene::slot_sceneChanged(const QList<QRectF> &region)
{
    emit signal_itemsCount(items().size());
}

void AssemblyGraphicsScene::mousePressEvent(QGraphicsSceneMouseEvent *event)
{
    QList<QGraphicsItem *> subItems = items(event->scenePos(), Qt::IntersectsItemShape, Qt::AscendingOrder);
    int subItemsLength = subItems.size();


    if ( subItemsLength == 0) {
        qDebug() << "RIEN DESSOUS...";
        _pipeItem->clear();
        if (this == _pipeItem.data()->scene()) {
            removeItem(_pipeItem);
        }
        return;
    }
    QGraphicsItem * subItem = subItems[0];

    qDebug() << "ITEM SRC" << subItem;

    if ((!subItem) || (subItem->type() <= QGraphicsItem::UserType)/* || (subItem->zValue() > 0)*/) {
        return;
    }

    ElementWidget * elt = (ElementWidget *)(subItem);
    int eltType = elt->type();
    qDebug() << "Type=" << eltType;

    if ((eltType == SourceType) || (eltType == ProcessorType)|| (eltType == DestinationType)) {
        // Affichage uniquement sur bouton gauche...
        if (!(event->buttons() & Qt::LeftButton)) {
            // affichage popup suppression
            QMenu menu;
            QAction * delItemAct = menu.addAction(tr("Supprimer"));
            QAction * delSrcAct = NULL;
            QAction * delDestAct =NULL;
            if ((eltType == ProcessorType) || (eltType == DestinationType)) {
                delSrcAct = menu.addAction(tr("Supprimer les sources"));
            }
            if (eltType != DestinationType) {
                delDestAct = menu.addAction(tr("Supprimer les destinations"));
            }

            QAction * cmd = menu.exec(event->screenPos());

            if (!cmd) {
                return;
            }
            int delMode = 0;
            if (cmd == delItemAct) {
                // suppression item
                delMode = 3;
            }
            if ((cmd == delSrcAct) || (delMode & 1)) {
                // suppression sources
                foreach(PipeWidget * pipeWidget, _connectors) {
                    if (pipeWidget->getEndElement() == elt) {
                        _connectors.removeOne((PipeWidget *)pipeWidget);
                        pipeWidget->deleteLater();
                    }
                }

            }
            if ((cmd == delDestAct) || (delMode & 2)) {
                // suppression destinations
                foreach(PipeWidget * pipeWidget, _connectors) {
                    if (pipeWidget->getStartElement() == elt) {
                        _connectors.removeOne((PipeWidget *)pipeWidget);
                        pipeWidget->deleteLater();
                    }
                }
            }
            if (delMode == 3) {
                removeItem(elt);
                if ((_sourceWidget != elt) && (_destinationWidget != elt)) {
                    _processorsWidgets.remove(_processorsWidgets.key((ProcessorWidget *)elt));
                }
                elt->deleteLater();
                qDebug() << "Verif source = " << int(_sourceWidget.data());
                qDebug() << "Verif destination = " << int(_destinationWidget.data());
            }

            return;
        }
        int sourcePipe = elt->getOutputLine(event->scenePos());
        qDebug() << "Source et ligne" << (QGraphicsItem*)elt << sourcePipe;

        if (sourcePipe > -1) {
            if (elt->getOutputWidget(sourcePipe) != 0) {
                qDebug() << "Deja pris..";
                _pipeItem->clear();
                removeItem(_pipeItem);
                return;
            }
            addItem(_pipeItem);
            _pipeItem->setStart(true, elt, sourcePipe);
            _pipeItem->setVisible(true);
            qDebug() << "START...";
        }
        // }

    } else if (eltType == DestinationType) {
        qDebug() << "Destination...";

    } else if (eltType == PipeType) {
        if (event->buttons() & Qt::RightButton) {
            removeItem(elt);
            _connectors.removeOne((PipeWidget *)elt);
            delete elt;
        }
    } else if (eltType == ParametersType) {
        //_expertGui->_
        //emit signal_selectParameters(parameters);
    }

//    if ((_connectors.size() == 0) && (_parametersWidget == NULL) && (_sourceWidget == NULL) && (_processorsWidgets.size() == 0) && (_destinationWidget == NULL)) {
//        _mainGui->enableDeleteAssemby(false);
//    }
    QGraphicsScene::mousePressEvent(event);
}

void AssemblyGraphicsScene::mouseMoveEvent(QGraphicsSceneMouseEvent *event)
{
    bool handCursor = false;
    if (event->buttons() & Qt::LeftButton) {
        if (_pipeItem->hasStart()) {
            _pipeItem->toDestinationPos(event->scenePos());
            QList<QGraphicsItem *> subItems = items(event->scenePos(), Qt::IntersectsItemShape, Qt::AscendingOrder);
            if (subItems.size() > 0) {
                QGraphicsItem * subItem = subItems[0];
                int eltType = subItem->type();
                if (eltType > QGraphicsItem::UserType) {
                    ElementWidget * elt = (ElementWidget *)(subItem);
                    if ((eltType == ProcessorType) || (eltType == DestinationType)) {
                        int destinationPipe = elt->getInputLine(event->scenePos());
                        if (destinationPipe > -1) {
                            if (elt->getInputWidget(destinationPipe) == 0) {
                                handCursor = true;
                            }
                        }
                    }
                }
            }
            update();
        }

    } else if (int(event->buttons()) == Qt::NoButton) {
        QList<QGraphicsItem *> subItems = items(event->scenePos(), Qt::IntersectsItemShape, Qt::AscendingOrder);
        if (subItems.size() > 0) {
            QGraphicsItem * subItem = subItems[0];
            int eltType = subItem->type();
            if (eltType > QGraphicsItem::UserType) {
                ElementWidget * elt = (ElementWidget *)(subItem);
                if ((eltType == ProcessorType) || (eltType == SourceType)) {
                    int sourcePipe = elt->getOutputLine(event->scenePos());
                    if (sourcePipe > -1) {
                        handCursor = true;
                    }
                }
            }
        }
    } else {
        _pipeItem->clear();
        removeItem(_pipeItem);
    }
    //   }
    // }
    // }
    if (handCursor) {
        _viewport->setCursor(Qt::OpenHandCursor);
    } else {
        _viewport->setCursor(Qt::ArrowCursor);
    }
    QGraphicsScene::mouseMoveEvent(event);
}

void AssemblyGraphicsScene::mouseReleaseEvent(QGraphicsSceneMouseEvent *event)
{
    if (_pipeItem->getTempStartElement() == 0) {
        return;
    }

    QList<QGraphicsItem *> subItems = items(event->scenePos(), Qt::IntersectsItemShape, Qt::AscendingOrder);
    if (subItems.size() == 0) {
        _pipeItem->clear();
        removeItem(_pipeItem);
        update();
        return;
    }
    QGraphicsItem * subItem = subItems[0];

    qDebug() << "ITEM DEST" << subItem;

    if (!_pipeItem->getTempStartElement()) {
        subItem = 0;
    }

    if ((!subItem) || (subItem->type() <= QGraphicsItem::UserType)) {
        _pipeItem->clear();
        removeItem(_pipeItem);
        update();
        return;
    }

    ElementWidget * elt = (ElementWidget *)(subItem);
    int eltType = elt->type();

    if ((eltType == ProcessorType) || (eltType == DestinationType)) {
        int destinationPipe = elt->getInputLine(event->scenePos());

        qDebug() << "Passe end..." << destinationPipe;

        if (destinationPipe > -1) {
            if (elt->getInputWidget(destinationPipe) != 0) {
                qDebug() << "INPUT DEJA UTILISEE...";
                _pipeItem->clear();
                removeItem(_pipeItem);
                update();
                return;
            }

            PipeWidget * newPipe = new PipeWidget();
            newPipe->clone(_pipeItem);
            newPipe->setEnd(elt, destinationPipe);
            newPipe->setColor(PipeWidget::getNextColor());
            newPipe->toDestinationPos(elt->getInputLinePos(destinationPipe));
            addItem(newPipe);
            _connectors.append(newPipe);
            update();
        }

    } else if (eltType == ProcessorType) {
        qDebug() << "Processor...";

    } else if (eltType == DestinationType) {
        qDebug() << "Destination...";

    }

    _pipeItem->clear();
    removeItem(_pipeItem);

    _viewport->setCursor(Qt::ArrowCursor);
    QGraphicsScene::mouseReleaseEvent(event);
}

void AssemblyGraphicsScene::dragEnterEvent(QGraphicsSceneDragDropEvent *event){
    QString sourceName = event->source()->objectName();
    if (sourceName == "_TRW_parameters") {
        QTreeWidget * treeWid = qobject_cast<QTreeWidget *>( event->source());
        QTreeWidgetItem * parentItem = treeWid->currentItem()->parent();
        // Modif: on n'instancie qu'une instance, pas le modèle...
        if (!parentItem) {
            event->ignore();
            return;
        }
    }
    event->accept();
}

void AssemblyGraphicsScene::dropEvent(QGraphicsSceneDragDropEvent *event)
{
    //    qDebug() << "DROP SCENE";
    QWidget * source = event->source();
    QString sourceName = source->objectName();
    if (sourceName == "_TRW_assemblies") {
        qDebug() << "Assembly...";
        QTreeWidget * treeWid = qobject_cast<QTreeWidget *>(source);
        QTreeWidgetItem * item = treeWid->currentItem();
        QString assemblyName = item->text(0);
        loadAssembly(assemblyName);
    } else if (sourceName == "_TRW_parameters") {
        QTreeWidget * treeWid = qobject_cast<QTreeWidget *>(source);
        QTreeWidgetItem * item = treeWid->currentItem();
        QTreeWidgetItem * parentItem = item->parent();

        // Modif: on n'instancie qu'une instance, pas le modèle...
        // déjà traité dans le dragEnter...
        if (!parentItem) {
            event->ignore();
            return;
        }
        QString paramName = item->data(0, Qt::DisplayRole).toString();
        if (parentItem) {
            QString modelName = parentItem->data(0, Qt::DisplayRole).toString();
            paramName.prepend(modelName+ "/");
        }

        QPointer<ParametersWidget> param = _expertGui -> getParametersWidget(paramName);

        if (_parametersWidget != 0) {
            removeItem(_parametersWidget);
            delete _parametersWidget;
        }
        _parametersWidget = param;

        if (param) {
            addItem(param);
            qDebug() << "PARAM rec=" << param->boundingRect();
            param->setPos(200, 40);
            qDebug() << "Déposé en" << param->scenePos();
        }

    } else if (sourceName == "_LW_inputs") {
        QListWidget * listWid = qobject_cast<QListWidget *>(source);
        QListWidgetItem * item = listWid->currentItem();
        QString srcName = item->data(Qt::DisplayRole).toString();
        SourceWidget * src = _expertGui -> getSourceWidget(srcName);

        if (_sourceWidget != 0) {
            removeItem(_sourceWidget);
            // modification des connexions
            foreach(PipeWidget * pipe, _connectors) {
                if (pipe->getStartElement() == _sourceWidget.data()) {
                    _connectors.removeOne(pipe);
                    pipe->deleteLater();
                }
            }

            delete _sourceWidget;
        }
        _sourceWidget = src;

        if (src) {
            addItem(src);
            qDebug() << "SRC rec=" << src->boundingRect();
            src->setPos(0, 40);
            qDebug() << "Déposé en" << src->scenePos();
            qDebug() << "Scene rect:" << sceneRect();
        }


    } else if (sourceName == "_LW_processors") {
        QListWidget * listWid = qobject_cast<QListWidget *>(source);
        QListWidgetItem * item = listWid->currentItem();
        QString procName = item->data(Qt::DisplayRole).toString();
        ProcessorWidget * proc = _expertGui -> getProcessorWidget(procName);

        qDebug() << "PROC rec=" << proc->boundingRect();
        qDebug() << "Avant arrondi:" << event->scenePos().y();
        int posIndex = qMax(0, qRound(event->scenePos().y()/195.0)-1);
        qDebug() << "INDEX=" << posIndex;
        posIndex++;
        if (posIndex == 0) {
            posIndex = 1;
        }
        if (_processorsWidgets.contains(posIndex)) {
            QList<quint8> keys = _processorsWidgets.keys();
            while(!keys.isEmpty()) {
                quint8 curIndex = keys.takeLast();
                if (curIndex < posIndex) {
                    break;
                }
                ProcessorWidget * curProc = _processorsWidgets.value(curIndex);
                _processorsWidgets.remove(curIndex);
                _processorsWidgets.insert(curIndex+1, curProc);
                curProc->setPos(0, 195 * curIndex + 215);
                curProc->setOrder(curIndex+1);
            }
        }
        _processorsWidgets.insert(posIndex, proc);
        proc->setPos(0, 195 * (posIndex-1) + 215);
        proc->setOrder(posIndex);
        addItem(proc);

        // deplacement de la la destination si presente
        if (_destinationWidget != NULL) {
            quint8 destPos = _processorsWidgets.keys().last();
            _destinationWidget -> setOrder(destPos +1);
            _destinationWidget -> setPos(0, 195 * destPos + 215);
        }

    } else if (sourceName == "_LW_outputs") {
        QListWidget * listWid = qobject_cast<QListWidget *>(source);
        QListWidgetItem * item = listWid->currentItem();
        QString dstName = item->data(Qt::DisplayRole).toString();
        DestinationWidget * dst = _expertGui -> getDestinationWidget(dstName);

        if (dst) {
            quint32 posIndex = 1;

            if (_destinationWidget != NULL) {
                removeItem(_destinationWidget);
                // modification des connexions
                foreach(PipeWidget * pipe, _connectors) {
                    if (pipe->getStartElement() == _destinationWidget.data()) {
                        _connectors.removeOne(pipe);
                        pipe->deleteLater();
                    }
                }

                delete _destinationWidget;
            }
            _destinationWidget = dst;

            if (_processorsWidgets.size() > 0) {
                posIndex = _processorsWidgets.keys().last()+1;
            }
            addItem(dst);
            dst->setOrder(posIndex);
            dst->setPos(0, 195 * (posIndex-1) + 215);
        }
    }

//    _mainGui->enableDeleteAssemby();
    event->accept();
}

bool AssemblyGraphicsScene::event(QEvent *event)
{
    if (event->type() == QEvent::Enter) {
        qDebug() << "Enter mouse";
        _viewport->setCursor(Qt::ArrowCursor);
    } else if (event->type() == QEvent::Leave) {
        qDebug() << "Leave mouse";
    }

    return QGraphicsScene::event(event);
}

void AssemblyGraphicsScene::setExpertGui(ExpertFormWidget *gui) {
    _expertGui = gui;
    _viewport =  views().at(0)->viewport();
    // _viewport->setCursor(Qt::CrossCursor);

}

void AssemblyGraphicsScene::setMainGui(AssemblyGui *gui)
{
    _mainGui = gui;
    connect(this, SIGNAL(signal_itemsCount(int)), _mainGui, SLOT(slot_assemblyElementsCount(int)));
}

void AssemblyGraphicsScene::reset()
{
//    clear();

    if ( items().contains(_pipeItem.data())) {
        removeItem(_pipeItem);
    }
    _pipeItem->clear();

    foreach(PipeWidget * pipeWidget, _connectors) {
        if (pipeWidget) {
            removeItem(pipeWidget);
            pipeWidget->deleteLater();
        }
    }
    _connectors.clear();

    if (_parametersWidget) {
        removeItem(_parametersWidget);
        _parametersWidget.data()->deleteLater();
    }

    if (_sourceWidget) {
        removeItem(_sourceWidget);
        _sourceWidget.data()->deleteLater();
    }

    if (_destinationWidget) {
        removeItem(_destinationWidget);
        _destinationWidget.data()->deleteLater();
    }

    foreach(ProcessorWidget * processor, _processorsWidgets) {
        if (processor) {
            removeItem(processor);
            processor->deleteLater();
        }
    }
    _processorsWidgets.clear();


}

bool AssemblyGraphicsScene::saveAssembly(QString filename, QString name, KeyValueList fields)
{
    // TODO: vérification, de la présence des paramètres
    // TODO: vérification de la liste des entrées
    QFile assemblyFile(filename);
    QTextStream os(&assemblyFile);
    if (!assemblyFile.open(QIODevice::WriteOnly)) {
        qDebug() << "Erreur ouverture ecriture";
        return false;
    }

    bool valid = false;

    os << "<?xml version=\"1.0\" encoding=\"utf-8\"?>\n";
    os << QString("<MatisseAssembly name=\"%1\" usable=\"%2\">\n").arg(name).arg(valid);
    os << "\t<DescriptorFields>\n";
    QStringList fieldsKeys = QStringList() << "Author" << "Version" << "Comments" << "Date";
    foreach(QString key, fieldsKeys) {
        // test manuel pour l'instant...
        os << QString("\t\t<%1>\n\t\t\t%2\n\t\t</%1>\n").arg(key).arg(fields.getValue(key));
    }
    os << "\t</DescriptorFields>\n";
    // On teste tout au cas ou l'assemblage ne serait pas valide...
    // ecriture des parameters
    QString parametersName = "";
    QString parametersVersion = "";
    if (_parametersWidget) {
        QStringList args = _parametersWidget ->getName().split("\n");
        if (args.size() > 1) {
            parametersVersion = args[0];
            parametersName = args[1];
        }
    }
    os << QString("\t<Parameters id=\"%1\" model=\"%2\" name=\"%3\"/>\n").arg(99).arg(parametersVersion).arg(parametersName);
    os << "\n";
    // ecriture de la source
    QString sourceName = "";
    if (_sourceWidget) {
        sourceName = _sourceWidget->getName();
    }

    os << QString("\t<Source id=\"%1\" name=\"%2\" order=\"0\"/>\n").arg(99).arg(sourceName);
    os << "\n";
    // ecriture des processeurs
    os << "\t<Processors>\n";
    foreach(quint8 procPos, _processorsWidgets.keys()) {
        ProcessorWidget * curProc = _processorsWidgets.value(procPos);
        os << QString("\t\t<Processor id=\"%1\" name=\"%2\" order=\"%3\"/>\n").arg(99).arg(curProc->getName()).arg(procPos);
    }
    os << "\t</Processors>\n";
    os << "\n";

    // ecriture de la destination
    QString destinationName;
    QString destinationOrder;
    if (_destinationWidget) {
        destinationName = QString("%1").arg(_destinationWidget->getName());
        destinationOrder = QString("%1").arg(_destinationWidget->getOrder());
    }
    os << QString("\t<Destination id=\"%1\" name=\"%2\" order=\"%3\"/>\n").arg(99).arg(destinationName).arg(destinationOrder);
    os << "\n";
    // ecriture des relations
    os << "\t<Connections>\n";
    foreach(PipeWidget * pipe, _connectors) {
        qint8 order1 = pipe->getStartElement()->getOrder();
        qint8 line1 = pipe->getStartElementLine();
        qint8 order2 = pipe->getEndElement()->getOrder();
        qint8 line2 = pipe->getEndElementLine();
        QColor color = pipe->getColor();
        os << QString("\t\t<Connection startOrder=\"%1\" startLine=\"%2\" endOrder=\"%3\" endLine=\"%4\" color=\"%5\"/>\n").arg(order1).arg(line1).arg(order2).arg(line2).arg(color.rgba());
    }
    os << "\t</Connections>\n";
    os << "</MatisseAssembly>";
    os.flush();
    assemblyFile.close();

    return true;
}

bool AssemblyGraphicsScene::loadAssembly(QString assemblyName)
{
    qDebug() << "AssemblyGraphicsScene::loadAssembly" << assemblyName;

    AssemblyDefinition * assembly = _server->xmlTool().getAssembly(assemblyName);
    if (!assembly) {
        QMessageBox::warning(_mainGui, tr("Assemblage invalide"), tr("L'assemblage ne peut être chargé..."));
        return false;
    }

    reset();

    bool continueLoad = false;
    bool paramOk = false;
    QString partialLoadStr = tr("L'assemblage sera partiellement chargé...\nContinuer?");
    // recherche parametres
    ParameterDefinition * parameters = assembly->parametersDefinition();
    if (parameters) {

        QString parametersName = parameters->name();
        QString paramatersModel = parameters->model();

        qDebug() << "Model et name parameter:" << parametersName << paramatersModel;

        _parametersWidget = _expertGui -> getParametersWidget(paramatersModel + "/" + parametersName);
        if (_parametersWidget) {
            _parametersWidget->setPos(200, 40);
            addItem(_parametersWidget);
            paramOk = true;
        }
    }

    if (!paramOk){
        qWarning() << "Parameters NOK";
        if (!continueLoad) {
            continueLoad = (QMessageBox::question(_mainGui, tr("Paramètres invalides"),
                                                  partialLoadStr,
                                                  QMessageBox::Yes,
                                                  QMessageBox::No)
                            == QMessageBox::No);
            if (!continueLoad) {
                return false;
            }
        }
    }

    // recherche source
    bool srcOk = false;
    SourceDefinition * source = assembly->sourceDefinition();
    if (source) {

        QString sourceName = source->name();

        qDebug() << "Name source:" << sourceName;

        _sourceWidget = _expertGui -> getSourceWidget(sourceName);
        if (_sourceWidget) {
            _sourceWidget->setPos(0, 40);
            addItem(_sourceWidget);
            srcOk = true;
        }
    }

    if (!srcOk){
        qWarning() << "Source NOK";
        if (!continueLoad) {
            continueLoad = (QMessageBox::question(_mainGui, tr("Source invalide"),
                                                  partialLoadStr,
                                                  QMessageBox::Yes,
                                                  QMessageBox::No)
                            == QMessageBox::Yes);
            if (!continueLoad) {
                return false;
            }
        }
    }

    // recherche processors
    foreach(ProcessorDefinition * processor, assembly->processorDefs()) {
        if (processor) {
            QString procName = processor->name();
            quint32 procOrder = processor->order();
            qDebug() << "Processeur name, order" << procName << procOrder;
            ProcessorWidget * newProc = _expertGui -> getProcessorWidget(procName);
            if (newProc) {
                newProc->setOrder(procOrder);
                _processorsWidgets.insert(procOrder, newProc);
                newProc->setPos(0, 195 * (procOrder-1) + 215);
                addItem(newProc);
            } else {
                if (!continueLoad) {
                    continueLoad = (QMessageBox::question(_mainGui, "Processeur invalide",
                                                          partialLoadStr,
                                                          QMessageBox::Yes,
                                                          QMessageBox::No)
                                    == QMessageBox::Yes);
                    if (!continueLoad) {
                        return false;
                    }
                }
            }

        } else {
            if (!continueLoad) {
                continueLoad = (QMessageBox::question(_mainGui, tr("Processeur invalide"),
                                                      partialLoadStr,
                                                      QMessageBox::Yes,
                                                      QMessageBox::No)
                                == QMessageBox::Yes);
                if (!continueLoad) {
                    return false;
                }
            }
        }
    }

    // recherche de la destination
    DestinationDefinition * destination = assembly->destinationDefinition();
    bool destOk = false;
    if (assembly->destinationDefinition()) {
        QString destName = destination->name();
        quint32 destOrder = destination->order();
        qDebug() << "Destination name, order" << destName << destOrder;
        DestinationWidget * newDest = _expertGui -> getDestinationWidget(destName);
        if (newDest) {
            newDest->setOrder(destOrder);
            _destinationWidget = newDest;
            newDest->setPos(0, 195 * (destOrder-1) + 215);
            addItem(newDest);
            destOk =true;
        }
    }

    if (!destOk){
        if (!continueLoad) {
            continueLoad = (QMessageBox::question(_mainGui, tr("Destination invalide"),
                                                  partialLoadStr,
                                                  QMessageBox::Yes,
                                                  QMessageBox::No)
                            == QMessageBox::Yes);
            if (!continueLoad) {
                return false;
            }
        }
    }

    // recherche connections
    foreach(ConnectionDefinition * connection, assembly->connectionDefs()) {
        if (connection) {
            quint32 startOrder = connection->startOrder();
            quint32 startLine = connection->startLine();
            quint32 endOrder = connection->endOrder();
            quint32 endLine = connection->endLine();
            QRgb color = connection->color();

            qDebug() << "Connections startOrder, startLine, endOrder, endLine, color" << startOrder << startLine << endOrder << endLine << color;

            PipeWidget * newPipe = new PipeWidget();
            ElementWidget * startElt = NULL;
            ElementWidget * endElt = NULL;
            if (startOrder == 0) {
                startElt = _sourceWidget;
            } else {
                startElt = _processorsWidgets.value(startOrder, NULL);
            }
            endElt = _processorsWidgets.value(endOrder, NULL);
            if (endElt == NULL) {
                // on cherche si la sortie est une destination...
                if (_destinationWidget) {
                    if (_destinationWidget -> getOrder() == endOrder) {
                        endElt = _destinationWidget;
                    }
                }
            }
            if (startElt && endElt) {
                newPipe->setStart(true, startElt, startLine);
                newPipe->setEnd(endElt, endLine);
                newPipe->setColor(QColor(color));
                addItem(newPipe);
                _connectors.append(newPipe);

                PipeWidget::setCurrentColor(color);
                if (color != QColor().rgba()) {
                    PipeWidget::getNextColor();
                }
            }
        }
    }

    return true;
}

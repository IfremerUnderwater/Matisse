#include "AssemblyGraphicsScene.h"
#include "ExpertFormWidget.h"
#include "AssemblyGui.h"

using namespace MatisseServer;

AssemblyGraphicsScene::AssemblyGraphicsScene(const QRectF &sceneRect, QObject *parent) :
    QGraphicsScene(sceneRect, parent),
    _server(NULL), _processDataManager(NULL), _isSceneActive(false), _isAssemblyModified(false), _isAssemblyComplete(false)
{
    //    _startPos = QPointF();
    //    _endPos = QPointF();
    init();
}

void AssemblyGraphicsScene::init() {
    setSceneRect(0, 0, INACTIVE_SCENE_WIDTH, INACTIVE_SCENE_HEIGHT);
    _pipeItem = new PipeWidget();
    //    _pipeItem->setVisible(false);
    //    addItem(_pipeItem);
    _sourceWidget = NULL;
    _destinationWidget = NULL;
    //resolveMessageTargetWidget();
    connect(this, SIGNAL(changed(QList<QRectF>)), this, SLOT(slot_sceneChanged(QList<QRectF>)));
}

void AssemblyGraphicsScene::checkAssemblyComplete()
{
    // check source
    if (!_sourceWidget) {
        applyAssemblyCompleteness(false);
        return;
    }

    // check destination
    if (!_destinationWidget) {
        applyAssemblyCompleteness(false);
        return;
    }

    // check at least 1 processor
    if (_processorsWidgets.isEmpty()) {
        applyAssemblyCompleteness(false);
        return;
    }

    // check connectors
    if (_connectors.isEmpty()) {
        applyAssemblyCompleteness(false);
        return;
    }

    bool sourceConnected = false;
    bool destinationConnected = false;

    QSet<ProcessorWidget*> inputConnectedProcessors;
    QSet<ProcessorWidget*> outputConnectedProcessors;

    /* Parse connectors to determine what widgets are connected */
    foreach (PipeWidget *connector, _connectors) {
        ElementWidget * startElement = connector->getStartElement();
        ElementWidget * endElement = connector->getEndElement();

        if (startElement == _sourceWidget) {
            sourceConnected = true;
        } else {
            foreach (ProcessorWidget *processor, _processorsWidgets) {
                if (startElement == processor) {
                    inputConnectedProcessors.insert(processor);
                    break;
                }
            }
        }

        if (endElement == _destinationWidget) {
            destinationConnected = true;
        } else {
            foreach (ProcessorWidget *processor, _processorsWidgets) {
                if (endElement == processor) {
                    outputConnectedProcessors.insert(processor);
                    break;
                }
            }
        }
    }

    /* assembly is not complete if either source or destination is disconnected */
    if (!sourceConnected) {
        applyAssemblyCompleteness(false);
        return;
    }

    if (!destinationConnected) {
        applyAssemblyCompleteness(false);
        return;
    }

    /* check if all processors are connected at input and output */
    bool allProcessorsConnected = true;

    foreach (ProcessorWidget *processor, _processorsWidgets) {
        if (!inputConnectedProcessors.contains(processor)) {
            allProcessorsConnected = false;
            break;
        }

        if (!outputConnectedProcessors.contains(processor)) {
            allProcessorsConnected = false;
            break;
        }
    }

    applyAssemblyCompleteness(allProcessorsConnected);
}

void AssemblyGraphicsScene::applyAssemblyCompleteness(bool isComplete)
{
    bool hasChanged = (isComplete != _isAssemblyComplete);

    if (hasChanged) {
        _isAssemblyComplete = isComplete;
        AssemblyDefinition *assembly = NULL;
        if (isComplete) {
            assembly = new AssemblyDefinition();
        }

        emit signal_assemblyComplete(_isAssemblyComplete);
    }
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
                delSrcAct = menu.addAction(tr("Supprimer les connexions entrantes"));
            }
            if (eltType != DestinationType) {
                delDestAct = menu.addAction(tr("Supprimer les connexions sortantes"));
            }

            QAction * cmd = menu.exec(event->screenPos());

            if (!cmd) {
                return;
            }
            int delMode = 0;
            if (cmd == delItemAct) {
                // remove item
                delMode = 3;
            }
            if ((cmd == delSrcAct) || (delMode & 1)) {
                // remove incoming connexions
                foreach(PipeWidget * pipeWidget, _connectors) {
                    if (pipeWidget->getEndElement() == elt) {
                        _connectors.removeOne((PipeWidget *)pipeWidget);
                        pipeWidget->deleteLater();
                        _isAssemblyModified = true;

                    }
                }

            }
            if ((cmd == delDestAct) || (delMode & 2)) {
                // removing outgoing connexions
                foreach(PipeWidget * pipeWidget, _connectors) {
                    if (pipeWidget->getStartElement() == elt) {
                        _connectors.removeOne((PipeWidget *)pipeWidget);
                        pipeWidget->deleteLater();
                        _isAssemblyModified = true;
                    }
                }
            }
            if (delMode == 3) {
                removeItem(elt);
                // removing user for expected parameters
                _server->removeModuleAndExpectedParameters(elt->getName());

                if ((_sourceWidget != elt) && (_destinationWidget != elt)) {
                    _processorsWidgets.remove(_processorsWidgets.key((ProcessorWidget *)elt));
                }
                elt->deleteLater();
                _isAssemblyModified = true;
            }

            if (_isAssemblyModified) {
                checkAssemblyComplete();

                // notify modification
                emit signal_assemblyModified();
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

    } else if (eltType == PipeType) {
        if (event->buttons() & Qt::RightButton) {
            removeItem(elt);
            _connectors.removeOne((PipeWidget *)elt);
            delete elt;

            _isAssemblyModified = true;
            checkAssemblyComplete();
            emit signal_assemblyModified();
        }
    }

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

            _isAssemblyModified = true;
            checkAssemblyComplete();
            emit signal_assemblyModified();

        }

    }

    _pipeItem->clear();
    removeItem(_pipeItem);

    _viewport->setCursor(Qt::ArrowCursor);
    QGraphicsScene::mouseReleaseEvent(event);
}


void AssemblyGraphicsScene::dropEvent(QGraphicsSceneDragDropEvent *event)
{
    if (!_isSceneActive) {
        /* resize scene so that scrollbar is available */
        setSceneRect(0, 0, ACTIVE_SCENE_WIDTH, ACTIVE_SCENE_HEIGHT);
        _isSceneActive;
    }

    //    qDebug() << "DROP SCENE";
    QWidget * source = event->source();
    QString sourceName = source->objectName();

    if (sourceName == "_TRW_assemblies") {
        qDebug() << "Assembly...";
        QTreeWidget * treeWid = qobject_cast<QTreeWidget *>(source);
        QTreeWidgetItem * item = treeWid->currentItem();
        QString assemblyName = item->text(0);
        loadAssembly(assemblyName);

    } else if (sourceName == "_LW_inputs") {
        if (_sourceWidget != 0) {            
            // remove parameters expected by the module
            _server->removeModuleAndExpectedParameters(_sourceWidget->getName());

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

        QListWidget * listWid = qobject_cast<QListWidget *>(source);
        QListWidgetItem * item = listWid->currentItem();
        QString srcName = item->data(Qt::DisplayRole).toString();
        SourceWidget * src = _elementProvider -> getSourceWidget(srcName);

        if (!src) {
            qCritical() << QString("Source not found in repository for item '%1'").arg(item->text());
            event->ignore();
        }

        _sourceWidget = src;

        if (src) {
            addItem(src);
            qDebug() << "SRC rec=" << src->boundingRect();
            src->setPos(0, 40);
            qDebug() << "Déposé en" << src->scenePos();
            qDebug() << "Scene rect:" << sceneRect();
        }

        _isAssemblyModified = true;

    } else if (sourceName == "_LW_processors") {
        QListWidget * listWid = qobject_cast<QListWidget *>(source);
        QListWidgetItem * item = listWid->currentItem();
        QString procName = item->data(Qt::DisplayRole).toString();
        ProcessorWidget * proc = _elementProvider -> getProcessorWidget(procName);

        if (!proc) {
            qCritical() << QString("Processor not found in repository for item '%1'").arg(item->text());
            event->ignore();
        }

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

        _isAssemblyModified = true;

    } else if (sourceName == "_LW_outputs") {

        quint32 posIndex = 1;

        if (_destinationWidget != NULL) {
            // remove expected parameters for destination module
            _server->removeModuleAndExpectedParameters(_destinationWidget->getName());

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

        QListWidget * listWid = qobject_cast<QListWidget *>(source);

        QListWidgetItem * item = listWid->currentItem();
        QString dstName = item->data(Qt::DisplayRole).toString();
        DestinationWidget * dst = _elementProvider -> getDestinationWidget(dstName);

        if (!dst) {
            qCritical() << QString("Destination not found in repository for item '%1'").arg(item->text());
            event->ignore();
        }

        _destinationWidget = dst;
        if (_processorsWidgets.size() > 0) {
            posIndex = _processorsWidgets.keys().last()+1;
        }

        addItem(dst);
        dst->setOrder(posIndex);
        dst->setPos(0, 195 * (posIndex-1) + 215);

        _isAssemblyModified = true;
    }

    if (_isAssemblyModified) {
        checkAssemblyComplete();

        // notify modification
        emit signal_assemblyModified();
    }

    event->accept();
}

bool AssemblyGraphicsScene::event(QEvent *event)
{
    if (event->type() == QEvent::Enter) {
        //qDebug() << "Enter mouse";
        _viewport->setCursor(Qt::ArrowCursor);
    } else if (event->type() == QEvent::Leave) {
        //qDebug() << "Leave mouse";
    }

    return QGraphicsScene::event(event);
}

/* Link to the UI element that provides element widgets (source, processors, destination) */
void AssemblyGraphicsScene::setElementWidgetProvider(ElementWidgetProvider *elementProvider)
{
    _elementProvider = elementProvider;
}

void AssemblyGraphicsScene::reset()
{
    // clear parameters view
    _server->parametersManager()->clearExpectedParameters();

    // clear scene
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

    /* resize scene to hide scrollbar */
    setSceneRect(0, 0, INACTIVE_SCENE_WIDTH, INACTIVE_SCENE_HEIGHT);
    _isSceneActive = false;
    _isAssemblyModified = false;
    applyAssemblyCompleteness(false);
}

bool AssemblyGraphicsScene::saveAssembly(QString filename, AssemblyDefinition *assembly)
{
    /* Assembly completeness is not tested here : the completeness event is signalled to
     * controlling class that should handle save action activation or not */

    if (!assembly) {
        // This case can not occur as assembly is tested by calling class
        qWarning() << "Assembly is null : could not be saved";
        return false;
    }    

    // TODO: vérification, de la présence des paramètres
    // TODO: vérification de la liste des entrées
    QFile assemblyFile(filename);
    QTextStream os(&assemblyFile);
    os.setCodec("UTF-8"); // forcer l'encodage en UTF-8, sinon les caractères accentués
                          // mal encodés empêchent de relire l'assemblage
    if (!assemblyFile.open(QIODevice::WriteOnly)) {
        qDebug() << "Erreur ouverture ecriture";
        return false;
    }

    bool valid = false;

    QString name = assembly->name();

    os << "<?xml version=\"1.0\" encoding=\"utf-8\"?>\n";
    os << QString("<MatisseAssembly name=\"%1\" isRealTime=\"%2\" usable=\"%3\">\n").arg(name).arg(assembly->isRealTime()).arg(valid);
    os << "\t<DescriptorFields>\n";

    os << QString("\t\t<%1>\n\t\t\t%2\n\t\t</%1>\n").arg("Author").arg(assembly->author());
    os << QString("\t\t<%1>\n\t\t\t%2\n\t\t</%1>\n").arg("Version").arg(assembly->version());
    os << QString("\t\t<%1>\n\t\t\t%2\n\t\t</%1>\n").arg("Comments").arg(assembly->comment());
    os << QString("\t\t<%1>\n\t\t\t%2\n\t\t</%1>\n").arg("Date").arg(assembly->date());

//    QStringList fieldsKeys = QStringList() << "Author" << "Version" << "Comments" << "Date";
//    foreach(QString key, fieldsKeys) {
//        // test manuel pour l'instant...
//        os << QString("\t\t<%1>\n\t\t\t%2\n\t\t</%1>\n").arg(key).arg(fields.getValue(key));
//    }
    os << "\t</DescriptorFields>\n";
    // On teste tout au cas ou l'assemblage ne serait pas valide...
    // ecriture des parameters
//    QString parametersName = "";
//    QString parametersVersion = "";
//    if (_parametersWidget) {
//        QStringList args = _parametersWidget ->getName().split("\n");
//        if (args.size() > 1) {
//            parametersVersion = args[0];
//            parametersName = args[1];
//        }
//    }
//    os << QString("\t<Parameters id=\"%1\" model=\"%2\" name=\"%3\"/>\n").arg(99).arg(parametersVersion).arg(parametersName);
//    os << "\n";
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

    AssemblyDefinition * assembly = _processDataManager->getAssembly(assemblyName);
    if (!assembly) {
        QMessageBox::warning(_messageTargetWidget, tr("Assemblage invalide"), tr("L'assemblage ne peut etre charge..."));
        return false;
    }

    reset();

    /* resize scene so that scrollbar is available */
    setSceneRect(0, 0, ACTIVE_SCENE_WIDTH, ACTIVE_SCENE_HEIGHT);
    _isSceneActive = true;

    bool continueLoad = true;
//    bool paramOk = true;
    QString partialLoadMessage = tr("L'assemblage sera partiellement charge...\nContinuer ?");

    // recherche source
    bool srcOk = false;
    SourceDefinition * source = assembly->sourceDefinition();
    if (source) {

        QString sourceName = source->name();

        qDebug() << "Name source:" << sourceName;

        _sourceWidget = _elementProvider -> getSourceWidget(sourceName);
        if (_sourceWidget) {
            _sourceWidget->setPos(0, 40);
            addItem(_sourceWidget);
            srcOk = true;
        }
    }

    if (!srcOk){
        qWarning() << "Source NOK";
        if (!continueLoad) {
            continueLoad = (QMessageBox::question(_messageTargetWidget, tr("Source invalide"),
                                                  partialLoadMessage,
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
            ProcessorWidget * newProc = _elementProvider -> getProcessorWidget(procName);
            if (newProc) {
                newProc->setOrder(procOrder);
                _processorsWidgets.insert(procOrder, newProc);
                newProc->setPos(0, 195 * (procOrder-1) + 215);
                addItem(newProc);
            } else {
                if (!continueLoad) {
                    continueLoad = (QMessageBox::question(_messageTargetWidget, "Processeur invalide",
                                                          partialLoadMessage,
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
                continueLoad = (QMessageBox::question(_messageTargetWidget, tr("Processeur invalide"),
                                                      partialLoadMessage,
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
        DestinationWidget * newDest = _elementProvider -> getDestinationWidget(destName);
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
            continueLoad = (QMessageBox::question(_messageTargetWidget, tr("Destination invalide"),
                                                  partialLoadMessage,
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

    checkAssemblyComplete();

    return true;
}

void AssemblyGraphicsScene::setMessageTarget(QWidget *targetWidget)
{
    _messageTargetWidget = targetWidget;
}

void AssemblyGraphicsScene::initViewport()
{
    _viewport =  views().at(0)->viewport();
}
void AssemblyGraphicsScene::setProcessDataManager(ProcessDataManager *processDataManager)
{
    _processDataManager = processDataManager;
}


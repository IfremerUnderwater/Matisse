#include "assembly_graphics_scene.h"
#include "assembly_editor.h"
#include "main_gui.h"

namespace matisse {

AssemblyGraphicsScene::AssemblyGraphicsScene(const QRectF &_scene_rect, QObject *_parent) :
    QGraphicsScene(_scene_rect, _parent),
    m_engine(NULL),
    m_is_scene_active(false),
    m_is_assembly_modified(false),
    m_is_assembly_complete(false)
{
    init();
}

void AssemblyGraphicsScene::init() {
    setSceneRect(0, 0, INACTIVE_SCENE_WIDTH, INACTIVE_SCENE_HEIGHT);
    m_pipe_item = new PipeWidget();
    m_source_widget = NULL;
    m_destination_widget = NULL;
    connect(this, SIGNAL(changed(QList<QRectF>)), this, SLOT(sl_sceneChanged(QList<QRectF>)));
}

void AssemblyGraphicsScene::checkAssemblyComplete()
{
    // check source
    if (!m_source_widget) {
        applyAssemblyCompleteness(false);
        return;
    }

    // check destination
    if (!m_destination_widget) {
        applyAssemblyCompleteness(false);
        return;
    }

    // check at least 1 processor
    if (m_processors_widgets.isEmpty()) {
        applyAssemblyCompleteness(false);
        return;
    }

    // check connectors
    if (m_connectors.isEmpty()) {
        applyAssemblyCompleteness(false);
        return;
    }

    bool source_connected = false;
    bool destination_connected = false;

    QSet<ProcessorWidget*> input_connected_processors;
    QSet<ProcessorWidget*> output_connected_processors;

    /* Parse connectors to determine what widgets are connected */
    foreach (PipeWidget *connector, m_connectors) {
        ElementWidget * start_element = connector->getStartElement();
        ElementWidget * end_element = connector->getEndElement();

        if (start_element == m_source_widget) {
            source_connected = true;
        } else {
            foreach (ProcessorWidget *processor, m_processors_widgets) {
                if (start_element == processor) {
                    input_connected_processors.insert(processor);
                    break;
                }
            }
        }

        if (end_element == m_destination_widget) {
            destination_connected = true;
        } else {
            foreach (ProcessorWidget *processor, m_processors_widgets) {
                if (end_element == processor) {
                    output_connected_processors.insert(processor);
                    break;
                }
            }
        }
    }

    /* assembly is not complete if either source or destination is disconnected */
    if (!source_connected) {
        applyAssemblyCompleteness(false);
        return;
    }

    if (!destination_connected) {
        applyAssemblyCompleteness(false);
        return;
    }

    /* check if all processors are connected at input and output */
    bool all_processors_connected = true;

    foreach (ProcessorWidget *processor, m_processors_widgets) {
        if (!input_connected_processors.contains(processor)) {
            all_processors_connected = false;
            break;
        }

        if (!output_connected_processors.contains(processor)) {
            all_processors_connected = false;
            break;
        }
    }

    applyAssemblyCompleteness(all_processors_connected);
}

void AssemblyGraphicsScene::applyAssemblyCompleteness(bool _is_complete)
{
    bool has_changed = (_is_complete != m_is_assembly_complete);

    if (has_changed) {
        m_is_assembly_complete = _is_complete;
        AssemblyDefinition *assembly = NULL;
        if (_is_complete) {
            assembly = new AssemblyDefinition();
        }

        Q_UNUSED(assembly) // Warningggggg !!!!!!!!!!!!!!!!!!!!!!!!!!!!

        emit si_assemblyComplete(m_is_assembly_complete);
    }
}

void AssemblyGraphicsScene::sl_sceneChanged(const QList<QRectF> &_region)
{
    Q_UNUSED(_region)
    emit si_itemsCount(items().size());
}

void AssemblyGraphicsScene::mousePressEvent(QGraphicsSceneMouseEvent *_event)
{
    QList<QGraphicsItem *> sub_items = items(_event->scenePos(), Qt::IntersectsItemShape, Qt::AscendingOrder);
    int sub_items_length = sub_items.size();


    if ( sub_items_length == 0) {
        qDebug() << "RIEN DESSOUS...";
        m_pipe_item->clear();
        if (this == m_pipe_item.data()->scene()) {
            removeItem(m_pipe_item);
        }
        return;
    }
    QGraphicsItem * sub_item = sub_items[0];

    qDebug() << "ITEM SRC" << sub_item;

    if ((!sub_item) || (sub_item->type() <= QGraphicsItem::UserType)/* || (subItem->zValue() > 0)*/) {
        return;
    }

    ElementWidget * elt = (ElementWidget *)(sub_item);
    int elt_type = elt->type();
    qDebug() << "Type=" << elt_type;

    if ((elt_type == SOURCE_TYPE) || (elt_type == PROCESSOR_TYPE)|| (elt_type == DESTINATION_TYPE)) {
        // Affichage uniquement sur bouton gauche...
        if (!(_event->buttons() & Qt::LeftButton)) {
            // affichage popup suppression
            QMenu menu;
            QAction * del_item_act = menu.addAction(tr("Delete"));
            QAction * del_src_act = NULL;
            QAction * del_dest_act =NULL;
            if ((elt_type == PROCESSOR_TYPE) || (elt_type == DESTINATION_TYPE)) {
                del_src_act = menu.addAction(tr("Delete incoming connexions"));
            }
            if (elt_type != DESTINATION_TYPE) {
                del_dest_act = menu.addAction(tr("Delete outgoing connexions"));
            }

            QAction * cmd = menu.exec(_event->screenPos());

            if (!cmd) {
                return;
            }
            int del_mode = 0;
            if (cmd == del_item_act) {
                // remove item
                del_mode = 3;
            }
            if ((cmd == del_src_act) || (del_mode & 1)) {
                // remove incoming connexions
                foreach(PipeWidget * pipe_widget, m_connectors) {
                    if (pipe_widget->getEndElement() == elt) {
                        m_connectors.removeOne((PipeWidget *)pipe_widget);
                        pipe_widget->deleteLater();
                        m_is_assembly_modified = true;

                    }
                }

            }
            if ((cmd == del_dest_act) || (del_mode & 2)) {
                // removing outgoing connexions
                foreach(PipeWidget * pipe_widget, m_connectors) {
                    if (pipe_widget->getStartElement() == elt) {
                        m_connectors.removeOne((PipeWidget *)pipe_widget);
                        pipe_widget->deleteLater();
                        m_is_assembly_modified = true;
                    }
                }
            }
            if (del_mode == 3) {
                removeItem(elt);
                // removing user for expected parameters
                m_engine->removeModuleAndExpectedParameters(elt->getName());

                if ((m_source_widget != elt) && (m_destination_widget != elt)) {
                    m_processors_widgets.remove(m_processors_widgets.key((ProcessorWidget *)elt));
                }
                elt->deleteLater();
                m_is_assembly_modified = true;
            }

            if (m_is_assembly_modified) {
                checkAssemblyComplete();

                // notify modification
                emit si_assemblyModified();
            }

            return;
        }

        int source_pipe = elt->getOutputLine(_event->scenePos());
        qDebug() << "Source et ligne" << (QGraphicsItem*)elt << source_pipe;

        if (source_pipe > -1) {
            if (elt->getOutputWidget(source_pipe) != 0) {
                qDebug() << "Deja pris..";
                m_pipe_item->clear();
                removeItem(m_pipe_item);
                return;
            }
            addItem(m_pipe_item);
            m_pipe_item->setStart(true, elt, source_pipe);
            m_pipe_item->setVisible(true);
            qDebug() << "START...";
        }
        // }

    } else if (elt_type == PIPE_TYPE) {
        if (_event->buttons() & Qt::RightButton) {
            removeItem(elt);
            m_connectors.removeOne((PipeWidget *)elt);
            delete elt;

            m_is_assembly_modified = true;
            checkAssemblyComplete();
            emit si_assemblyModified();
        }
    }

    QGraphicsScene::mousePressEvent(_event);
}

void AssemblyGraphicsScene::mouseMoveEvent(QGraphicsSceneMouseEvent *_event)
{
    bool hand_cursor = false;
    if (_event->buttons() & Qt::LeftButton) {
        if (m_pipe_item->hasStart()) {
            m_pipe_item->toDestinationPos(_event->scenePos());
            QList<QGraphicsItem *> sub_items = items(_event->scenePos(), Qt::IntersectsItemShape, Qt::AscendingOrder);
            if (sub_items.size() > 0) {
                QGraphicsItem * sub_item = sub_items[0];
                int elt_type = sub_item->type();
                if (elt_type > QGraphicsItem::UserType) {
                    ElementWidget * elt = (ElementWidget *)(sub_item);
                    if ((elt_type == PROCESSOR_TYPE) || (elt_type == DESTINATION_TYPE)) {
                        int destination_pipe = elt->getInputLine(_event->scenePos());
                        if (destination_pipe > -1) {
                            if (elt->getInputWidget(destination_pipe) == 0) {
                                hand_cursor = true;
                            }
                        }
                    }
                }
            }
            update();
        }

    } else if (int(_event->buttons()) == Qt::NoButton) {
        QList<QGraphicsItem *> sub_items = items(_event->scenePos(), Qt::IntersectsItemShape, Qt::AscendingOrder);
        if (sub_items.size() > 0) {
            QGraphicsItem * sub_item = sub_items[0];
            int elt_type = sub_item->type();
            if (elt_type > QGraphicsItem::UserType) {
                ElementWidget * elt = (ElementWidget *)(sub_item);
                if ((elt_type == PROCESSOR_TYPE) || (elt_type == SOURCE_TYPE)) {
                    int sourcePipe = elt->getOutputLine(_event->scenePos());
                    if (sourcePipe > -1) {
                        hand_cursor = true;
                    }
                }
            }
        }
    } else {
        m_pipe_item->clear();
        removeItem(m_pipe_item);
    }
    //   }
    // }
    // }
    if (hand_cursor) {
        m_viewport->setCursor(Qt::OpenHandCursor);
    } else {
        m_viewport->setCursor(Qt::ArrowCursor);
    }
    QGraphicsScene::mouseMoveEvent(_event);
}

void AssemblyGraphicsScene::mouseReleaseEvent(QGraphicsSceneMouseEvent *_event)
{
    if (m_pipe_item->getTempStartElement() == 0) {
        return;
    }

    QList<QGraphicsItem *> sub_items = items(_event->scenePos(), Qt::IntersectsItemShape, Qt::AscendingOrder);
    if (sub_items.size() == 0) {
        m_pipe_item->clear();
        removeItem(m_pipe_item);
        update();
        return;
    }
    QGraphicsItem * sub_item = sub_items[0];

    qDebug() << "ITEM DEST" << sub_item;

    if (!m_pipe_item->getTempStartElement()) {
        sub_item = 0;
    }

    if ((!sub_item) || (sub_item->type() <= QGraphicsItem::UserType)) {
        m_pipe_item->clear();
        removeItem(m_pipe_item);
        update();
        return;
    }

    ElementWidget * elt = (ElementWidget *)(sub_item);
    int elt_type = elt->type();

    if ((elt_type == PROCESSOR_TYPE) || (elt_type == DESTINATION_TYPE)) {
        int destination_pipe = elt->getInputLine(_event->scenePos());

        qDebug() << "Passe end..." << destination_pipe;

        if (destination_pipe > -1) {
            if (elt->getInputWidget(destination_pipe) != 0) {
                qDebug() << "INPUT DEJA UTILISEE...";
                m_pipe_item->clear();
                removeItem(m_pipe_item);
                update();
                return;
            }

            PipeWidget * new_pipe = new PipeWidget();
            new_pipe->clone(m_pipe_item);
            new_pipe->setEnd(elt, destination_pipe);
            new_pipe->setColor(PipeWidget::getNextColor());
            new_pipe->toDestinationPos(elt->getInputLinePos(destination_pipe));
            addItem(new_pipe);
            m_connectors.append(new_pipe);
            update();

            m_is_assembly_modified = true;
            checkAssemblyComplete();
            emit si_assemblyModified();

        }

    }

    m_pipe_item->clear();
    removeItem(m_pipe_item);

    m_viewport->setCursor(Qt::ArrowCursor);
    QGraphicsScene::mouseReleaseEvent(_event);
}


void AssemblyGraphicsScene::dropEvent(QGraphicsSceneDragDropEvent *_event)
{
    if (!m_is_scene_active) {
        /* resize scene so that scrollbar is available */
        setSceneRect(0, 0, ACTIVE_SCENE_WIDTH, ACTIVE_SCENE_HEIGHT);
        m_is_scene_active=true;
    }

    //    qDebug() << "DROP SCENE";
    QWidget * source = _event->source();
    QString source_name = source->objectName();

    if (source_name == "_TRW_assemblies") {
        qDebug() << "Assembly...";
        QTreeWidget * tree_wid = qobject_cast<QTreeWidget *>(source);
        QTreeWidgetItem * item = tree_wid->currentItem();
        QString assembly_name = item->text(0);
        loadAssembly(assembly_name);

    } else if (source_name == "_LW_inputs") {
        if (m_source_widget != 0) {
            // remove parameters expected by the module
            m_engine->removeModuleAndExpectedParameters(m_source_widget->getName());

            removeItem(m_source_widget);
            // modification des connexions
            foreach(PipeWidget * pipe, m_connectors) {
                if (pipe->getStartElement() == m_source_widget.data()) {
                    m_connectors.removeOne(pipe);
                    pipe->deleteLater();
                }
            }

            delete m_source_widget;

        }

        QListWidget * list_wid = qobject_cast<QListWidget *>(source);
        QListWidgetItem * item = list_wid->currentItem();
        QString src_name = item->data(Qt::DisplayRole).toString();
        SourceWidget * src = m_element_provider -> getSourceWidget(src_name);

        if (!src) {
            qCritical() << QString("Source not found in repository for item '%1'").arg(item->text());
            _event->ignore();
        }

        m_source_widget = src;

        if (src) {
            addItem(src);
            qDebug() << "SRC rec=" << src->boundingRect();
            src->setPos(0, 40);
            qDebug() << "Déposé en" << src->scenePos();
            qDebug() << "Scene rect:" << sceneRect();
        }

        m_is_assembly_modified = true;

    } else if (source_name == "_LW_processors") {
        QListWidget * list_wid = qobject_cast<QListWidget *>(source);
        QListWidgetItem * item = list_wid->currentItem();
        QString proc_name = item->data(Qt::DisplayRole).toString();
        ProcessorWidget * proc = m_element_provider -> getProcessorWidget(proc_name);

        if (!proc) {
            qCritical() << QString("Processor not found in repository for item '%1'").arg(item->text());
            _event->ignore();
        }

        qDebug() << "PROC rec=" << proc->boundingRect();
        qDebug() << "Avant arrondi:" << _event->scenePos().y();
        int pos_index = qMax(0, qRound(_event->scenePos().y()/195.0)-1);
        qDebug() << "INDEX=" << pos_index;
        pos_index++;
        if (pos_index == 0) {
            pos_index = 1;
        }
        if (m_processors_widgets.contains(pos_index)) {
            QList<quint8> keys = m_processors_widgets.keys();
            while(!keys.isEmpty()) {
                quint8 cur_index = keys.takeLast();
                if (cur_index < pos_index) {
                    break;
                }
                ProcessorWidget * cur_proc = m_processors_widgets.value(cur_index);
                m_processors_widgets.remove(cur_index);
                m_processors_widgets.insert(cur_index+1, cur_proc);
                cur_proc->setPos(0, 195 * cur_index + 215);
                cur_proc->setOrder(cur_index+1);
            }
        }
        m_processors_widgets.insert(pos_index, proc);
        proc->setPos(0, 195 * (pos_index-1) + 215);
        proc->setOrder(pos_index);
        addItem(proc);

        // deplacement de la la destination si presente
        if (m_destination_widget != NULL) {
            quint8 dest_pos = m_processors_widgets.keys().last();
            m_destination_widget -> setOrder(dest_pos +1);
            m_destination_widget -> setPos(0, 195 * dest_pos + 215);
        }

        m_is_assembly_modified = true;

    } else if (source_name == "_LW_outputs") {

        quint32 pos_index = 1;

        if (m_destination_widget != NULL) {
            // remove expected parameters for destination module
            m_engine->removeModuleAndExpectedParameters(m_destination_widget->getName());

            removeItem(m_destination_widget);
            // modification des connexions
            foreach(PipeWidget * pipe, m_connectors) {
                if (pipe->getStartElement() == m_destination_widget.data()) {
                    m_connectors.removeOne(pipe);
                    pipe->deleteLater();
                }
            }

            delete m_destination_widget;
        }

        QListWidget * list_wid = qobject_cast<QListWidget *>(source);

        QListWidgetItem * item = list_wid->currentItem();
        QString dst_name = item->data(Qt::DisplayRole).toString();
        DestinationWidget * dst = m_element_provider -> getDestinationWidget(dst_name);

        if (!dst) {
            qCritical() << QString("Destination not found in repository for item '%1'").arg(item->text());
            _event->ignore();
        }

        m_destination_widget = dst;
        if (m_processors_widgets.size() > 0) {
            pos_index = m_processors_widgets.keys().last()+1;
        }

        addItem(dst);
        dst->setOrder(pos_index);
        dst->setPos(0, 195 * (pos_index-1) + 215);

        m_is_assembly_modified = true;
    }

    if (m_is_assembly_modified) {
        checkAssemblyComplete();

        // notify modification
        emit si_assemblyModified();
    }

    _event->accept();
}

bool AssemblyGraphicsScene::event(QEvent *_event)
{
    if (_event->type() == QEvent::Enter) {
        //qDebug() << "Enter mouse";
        m_viewport->setCursor(Qt::ArrowCursor);
    } else if (_event->type() == QEvent::Leave) {
        //qDebug() << "Leave mouse";
    }

    return QGraphicsScene::event(_event);
}

/* Link to the UI element that provides element widgets (source, processors, destination) */
void AssemblyGraphicsScene::setElementWidgetProvider(ElementWidgetProvider *_element_provider)
{
    m_element_provider = _element_provider;
}

void AssemblyGraphicsScene::reset()
{
    // clear parameters view
    m_engine->parametersManager()->clearExpectedParameters();

    // clear scene
    if ( items().contains(m_pipe_item.data())) {
        removeItem(m_pipe_item);
    }
    m_pipe_item->clear();

    foreach(PipeWidget * pipe_widget, m_connectors) {
        if (pipe_widget) {
            removeItem(pipe_widget);
            pipe_widget->deleteLater();
        }
    }
    m_connectors.clear();

    if (m_source_widget) {
        removeItem(m_source_widget);
        m_source_widget.data()->deleteLater();
    }

    if (m_destination_widget) {
        removeItem(m_destination_widget);
        m_destination_widget.data()->deleteLater();
    }

    foreach(ProcessorWidget * processor, m_processors_widgets) {
        if (processor) {
            removeItem(processor);
            processor->deleteLater();
        }
    }
    m_processors_widgets.clear();

    /* resize scene to hide scrollbar */
    setSceneRect(0, 0, INACTIVE_SCENE_WIDTH, INACTIVE_SCENE_HEIGHT);
    m_is_scene_active = false;
    m_is_assembly_modified = false;
    applyAssemblyCompleteness(false);
}

/* Update assembly definition object with current graphical state */
void AssemblyGraphicsScene::updateAssembly(AssemblyDefinition *_assembly)
{
    _assembly->clearAllElements();

    SourceDefinition *source_def = new SourceDefinition(m_source_widget->getName());
    _assembly->setSourceDefinition(source_def);

    foreach(quint8 proc_pos, m_processors_widgets.keys()) {
        ProcessorWidget * cur_proc = m_processors_widgets.value(proc_pos);
        ProcessorDefinition *new_proc_def = new ProcessorDefinition(cur_proc->getName(), cur_proc->getOrder());
        _assembly->addProcessorDef(new_proc_def);
    }

    DestinationDefinition *destination_def = new DestinationDefinition(m_destination_widget->getName(), m_destination_widget->getOrder());
    _assembly->setDestinationDefinition(destination_def);

    foreach(PipeWidget * pipe, m_connectors) {
        qint8 order1 = pipe->getStartElement()->getOrder();
        qint8 line1 = pipe->getStartElementLine();
        qint8 order2 = pipe->getEndElement()->getOrder();
        qint8 line2 = pipe->getEndElementLine();
        QColor color = pipe->getColor();

        ConnectionDefinition *new_conn_def = new ConnectionDefinition(order1, line1, order2, line2, color.rgba());
        _assembly->addConnectionDef(new_conn_def);
    }
}

bool AssemblyGraphicsScene::loadAssembly(QString _assembly_name)
{
    qDebug() << "AssemblyGraphicsScene::loadAssembly" << _assembly_name;

    AssemblyDefinition * assembly = ProcessDataManager::instance()->getAssembly(_assembly_name);
    if (!assembly) {
        QMessageBox::warning(m_message_target_widget, tr("Invalid assembly"), tr("Assembly cannot be loaded..."));
        return false;
    }

    reset();

    /* resize scene so that scrollbar is available */
    setSceneRect(0, 0, ACTIVE_SCENE_WIDTH, ACTIVE_SCENE_HEIGHT);
    m_is_scene_active = true;

    bool continue_load = true;
//    bool paramOk = true;
    QString partial_load_message = tr("Assembly will only be partialy loaded...\nContinue ?");

    // recherche source
    bool src_ok = false;
    SourceDefinition * source = assembly->sourceDefinition();
    if (source) {

        QString source_name = source->name();

        qDebug() << "Name source:" << source_name;

        m_source_widget = m_element_provider -> getSourceWidget(source_name);
        if (m_source_widget) {
            m_source_widget->setPos(0, 40);
            addItem(m_source_widget);
            src_ok = true;
        }
    }

    if (!src_ok){
        qWarning() << "Source NOK";
        if (!continue_load) {
            continue_load = (QMessageBox::question(m_message_target_widget, tr("Invalid source"),
                                                  partial_load_message,
                                                  QMessageBox::Yes,
                                                  QMessageBox::No)
                            == QMessageBox::Yes);
            if (!continue_load) {
                return false;
            }
        }
    }

    // recherche processors
    foreach(ProcessorDefinition * processor, assembly->processorDefs()) {
        if (processor) {
            QString proc_name = processor->name();
            quint32 proc_order = processor->order();
            qDebug() << "Processeur name, order" << proc_name << proc_order;
            ProcessorWidget * new_proc = m_element_provider -> getProcessorWidget(proc_name);
            if (new_proc) {
                new_proc->setOrder(proc_order);
                m_processors_widgets.insert(proc_order, new_proc);
                new_proc->setPos(0, 195 * (proc_order-1) + 215);
                addItem(new_proc);
            } else {
                if (!continue_load) {
                    continue_load = (QMessageBox::question(m_message_target_widget, "Processeur invalide",
                                                          partial_load_message,
                                                          QMessageBox::Yes,
                                                          QMessageBox::No)
                                    == QMessageBox::Yes);
                    if (!continue_load) {
                        return false;
                    }
                }
            }

        } else {
            if (!continue_load) {
                continue_load = (QMessageBox::question(m_message_target_widget, tr("Invalid processor"),
                                                      partial_load_message,
                                                      QMessageBox::Yes,
                                                      QMessageBox::No)
                                == QMessageBox::Yes);
                if (!continue_load) {
                    return false;
                }
            }
        }
    }

    // recherche de la destination
    DestinationDefinition * destination = assembly->destinationDefinition();
    bool dest_ok = false;
    if (assembly->destinationDefinition()) {
        QString dest_name = destination->name();
        quint32 dest_order = destination->order();
        qDebug() << "Destination name, order" << dest_name << dest_order;
        DestinationWidget * new_dest = m_element_provider -> getDestinationWidget(dest_name);
        if (new_dest) {
            new_dest->setOrder(dest_order);
            m_destination_widget = new_dest;
            new_dest->setPos(0, 195 * (dest_order-1) + 215);
            addItem(new_dest);
            dest_ok =true;
        }
    }

    if (!dest_ok){
        if (!continue_load) {
            continue_load = (QMessageBox::question(m_message_target_widget, tr("Invalid destination"),
                                                  partial_load_message,
                                                  QMessageBox::Yes,
                                                  QMessageBox::No)
                            == QMessageBox::Yes);
            if (!continue_load) {
                return false;
            }
        }
    }

    // recherche connections
    foreach(ConnectionDefinition * connection, assembly->connectionDefs()) {
        if (connection) {
            quint32 start_order = connection->startOrder();
            quint32 start_line = connection->startLine();
            quint32 end_order = connection->endOrder();
            quint32 end_line = connection->endLine();
            QRgb color = connection->color();

            qDebug() << "Connections startOrder, startLine, endOrder, endLine, color" << start_order << start_line << end_order << end_line << color;

            PipeWidget * new_pipe = new PipeWidget();
            ElementWidget * start_elt = NULL;
            ElementWidget * end_elt = NULL;
            if (start_order == 0) {
                start_elt = m_source_widget;
            } else {
                start_elt = m_processors_widgets.value(start_order, NULL);
            }
            end_elt = m_processors_widgets.value(end_order, NULL);
            if (end_elt == NULL) {
                // on cherche si la sortie est une destination...
                if (m_destination_widget) {
                    if (m_destination_widget->getOrder() == (quint8)end_order) {
                        end_elt = m_destination_widget;
                    }
                }
            }
            if (start_elt && end_elt) {
                new_pipe->setStart(true, start_elt, start_line);
                new_pipe->setEnd(end_elt, end_line);
                new_pipe->setColor(QColor(color));
                addItem(new_pipe);
                m_connectors.append(new_pipe);

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

void AssemblyGraphicsScene::setMessageTarget(QWidget *_target_widget)
{
    m_message_target_widget = _target_widget;
}

void AssemblyGraphicsScene::initViewport()
{
    m_viewport =  views().at(0)->viewport();
}

} // namespace matisse



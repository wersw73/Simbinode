/*
 * Copyright © 2020 Gukova Anastasiia
 * Copyright © 2020 Gukov Anton <fexcron@gmail.com>
 *
 *
 * This file is part of Symbinode.
 *
 * Symbinode is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * Symbinode is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with Symbinode.  If not, see <https://www.gnu.org/licenses/>.
 */

#include "commands.h"
#include "node.h"
#include "edge.h"
#include "scene.h"
#include <iostream>

MoveCommand::MoveCommand(QList<QQuickItem *> nodes, QVector2D movVector, Frame *frame, Edge *edge, QUndoCommand *parent):
QUndoCommand(parent), m_nodes(nodes), m_movVector(movVector), m_frame(frame), m_intersectingEdge(edge)
{    
    for(auto item: m_nodes) {
        if(qobject_cast<Node*>(item)) {
            Node *node = qobject_cast<Node*>(item);
            m_newPos.push_back(QVector2D(node->baseX(), node->baseY()));
        }
        else if(qobject_cast<Frame*>(item)) {
            Frame *frame = qobject_cast<Frame*>(item);
            m_newPos.push_back(QVector2D(frame->baseX(), frame->baseY()));
        }
    }
    if(m_frame) {
        m_oldFrameX = m_frame->baseX();
        m_oldFrameY = m_frame->baseY();
        m_oldFrameWidth = m_frame->width();
        m_oldFrameHeight = m_frame->height();
    }
    if(m_intersectingEdge) {
        m_oldEndSocket = m_intersectingEdge->endSocket();
    }
}

MoveCommand::~MoveCommand() {
    m_nodes.clear();
}

void MoveCommand::undo() {
    for(auto item: m_nodes) {
        if(qobject_cast<Node*>(item)) {
            Node *node = qobject_cast<Node*>(item);
            if(m_frame && node->attachedFrame() == m_frame) {
                m_frame->removeItem(node);
                node->setAttachedFrame(nullptr);
            }
            node->setBaseX(node->baseX() - m_movVector.x());
            node->setBaseY(node->baseY() - m_movVector.y());
        }
        else if(qobject_cast<Frame*>(item)) {
            Frame *frame = qobject_cast<Frame*>(item);
            frame->setBaseX(frame->baseX() - m_movVector.x());
            frame->setBaseY(frame->baseY() - m_movVector.y());
        }
    }
    if(m_frame) {
        m_frame->setBaseX(m_oldFrameX);
        m_frame->setBaseY(m_oldFrameY);
        m_frame->setWidth(m_oldFrameWidth);
        m_frame->setHeight(m_oldFrameHeight);
    }
    if(m_intersectingEdge) {
        Node *node = qobject_cast<Node*>(m_nodes[0]);
        qobject_cast<Scene*>(node->parentItem())->deleteEdge(m_newEdge);
        m_newEdge->endSocket()->deleteEdge(m_newEdge);
        m_newEdge->startSocket()->deleteEdge(m_newEdge);
        m_newEdge->setParentItem(nullptr);
        m_intersectingEdge->endSocket()->deleteEdge(m_intersectingEdge);
        m_intersectingEdge->setEndSocket(m_oldEndSocket);
        m_oldEndSocket->addEdge(m_intersectingEdge);
        m_intersectingEdge->setEndPosition(m_oldEndSocket->globalPos());
        m_intersectingEdge->endSocket()->setValue(m_intersectingEdge->startSocket()->value());
    }
}

void MoveCommand::redo() {
    QList<QQuickItem*> nodesToFrame;
    for(int i = 0; i < m_nodes.count(); ++i) {
        QQuickItem *item = m_nodes[i];
        if(qobject_cast<Node*>(item)) {
            Node *n = qobject_cast<Node*>(item);
            n->setBaseX(m_newPos[i].x());
            n->setBaseY(m_newPos[i].y());
            if(m_frame && !n->attachedFrame()) nodesToFrame.push_back(n);
        }
        else if(qobject_cast<Frame*>(item)) {
            Frame *f = qobject_cast<Frame*>(item);
            f->setBaseX(m_newPos[i].x());
            f->setBaseY(m_newPos[i].y());
        }
    }
    if(m_frame) {
        m_frame->addNodes(nodesToFrame);
    }
    if(m_intersectingEdge) {
        m_oldEndSocket->deleteEdge(m_intersectingEdge);
        Node *node = qobject_cast<Node*>(m_nodes[0]);
        Socket *newEndSocket = node->getInputSocket(0);
        m_intersectingEdge->setEndSocket(newEndSocket);
        if(newEndSocket) {
            newEndSocket->addEdge(m_intersectingEdge);
            newEndSocket->setValue(m_intersectingEdge->startSocket()->value());
        }
        m_intersectingEdge->setEndPosition(newEndSocket->globalPos());
        Scene *scene = qobject_cast<Scene*>(node->parentItem());
        if(!m_newEdge) {
            m_newEdge = new Edge(scene);
            Socket *startSocket = node->getOutputSocket(0);
            m_newEdge->setStartSocket(startSocket);
            if(startSocket) startSocket->addEdge(m_newEdge);
            m_newEdge->setEndSocket(m_oldEndSocket);
            m_oldEndSocket->addEdge(m_newEdge);
            m_newEdge->setStartPosition(startSocket->globalPos());
            m_newEdge->setEndPosition(m_oldEndSocket->globalPos());
            m_oldEndSocket->setValue(m_newEdge->startSocket()->value());
        }
        else {
            m_newEdge->startSocket()->addEdge(m_newEdge);
            m_newEdge->endSocket()->addEdge(m_newEdge);
            m_newEdge->setStartPosition(m_newEdge->startSocket()->globalPos());
            m_newEdge->setEndPosition(m_newEdge->endSocket()->globalPos());
            m_newEdge->endSocket()->setValue(m_newEdge->startSocket()->value());
            m_newEdge->setParentItem(scene);
        }
        scene->addEdge(m_newEdge);
    }
}

AddNode::AddNode(Node *node, Scene *scene, QUndoCommand *parent): QUndoCommand (parent), m_scene(scene), m_node(node)
{
}

AddNode::~AddNode() {
    m_node = nullptr;
    m_scene = nullptr;
}

void AddNode::undo() {
    m_scene->deleteNode(m_node);
    m_node->setParentItem(nullptr);
}

void AddNode::redo() {
    m_scene->addNode(m_node);
    m_node->setParentItem(m_scene);
    m_node->generatePreview();
}

AddEdge::AddEdge(Edge *edge, Scene *scene, QUndoCommand *parent): QUndoCommand (parent), m_scene(scene), m_edge(edge)
{
}

AddEdge::~AddEdge() {
    m_edge = nullptr;
    m_scene = nullptr;
}

void AddEdge::undo() {
    m_scene->deleteEdge(m_edge);
    m_edge->startSocket()->deleteEdge(m_edge);
    m_edge->endSocket()->deleteEdge(m_edge);
    m_edge->setParentItem(nullptr);
}

void AddEdge::redo() {
    m_scene->addEdge(m_edge);
    m_edge->startSocket()->addEdge(m_edge);
    m_edge->endSocket()->addEdge(m_edge);
    m_edge->setStartPosition(m_edge->startSocket()->globalPos());
    m_edge->setEndPosition(m_edge->endSocket()->globalPos());
    m_edge->setParentItem(m_scene);
    m_edge->endSocket()->setValue(m_edge->startSocket()->value());
}

AddFrame::AddFrame(Frame *frame, Scene *scene, QUndoCommand *parent): QUndoCommand (parent), m_scene(scene), m_frame(frame)
{
    for(int i = 0; i < scene->countSelected(); ++i) {
        if(qobject_cast<Node*>(scene->atSelected(i))) {
            Node *node = qobject_cast<Node*>(scene->atSelected(i));
            m_nodes.push_back(QPair<Node*, Frame*>(node, node->attachedFrame()));
        }
    }
}

AddFrame::~AddFrame() {
    m_frame = nullptr;
    m_scene = nullptr;
}

void AddFrame::undo() {
    m_scene->deleteFrame(m_frame);
    m_frame->setParentItem(nullptr);
    for(auto p: m_nodes) {
        m_frame->removeItem(p.first);
        if(p.second) p.second->addNodes(QList<QQuickItem*>({p.first}));
    }
}

void AddFrame::redo() {
    m_scene->addFrame(m_frame);
    m_frame->setParentItem(m_scene);
    QList<QQuickItem*> addedNodes;
    for(auto p: m_nodes) {
        addedNodes.push_back(p.first);
        if(p.second) p.second->removeItem(p.first);
    }
    if(addedNodes.size() > 0) m_frame->addNodes(addedNodes);
}

DeleteCommand::DeleteCommand(QList<QQuickItem*> items, Scene *scene, bool saveConnection,
                             QUndoCommand *parent): QUndoCommand (parent), m_scene(scene), m_items(items),
    m_saveConnection(saveConnection){
}

DeleteCommand::~DeleteCommand() {
    m_items.clear();
    m_scene = nullptr;
}

void DeleteCommand::undo() {
    cancelDelete();
}

void DeleteCommand::redo() {
    if(m_saveConnection) deleteWithSaveConnection();
    else deleteBase();
}

void DeleteCommand::deleteBase() {
    for(auto item: m_items) {
        if(qobject_cast<Node*>(item)) {
            Node *node = qobject_cast<Node*>(item);
            m_scene->deleteNode(node);
            if(node->attachedFrame()) node->attachedFrame()->removeItem(node);
            node->setParentItem(nullptr);
        }
        else if(qobject_cast<Edge*>(item)) {
            Edge *edge = qobject_cast<Edge*>(item);
            m_scene->deleteEdge(edge);
            edge->startSocket()->deleteEdge(edge);
            edge->endSocket()->deleteEdge(edge);
            edge->setSelected(false);
            edge->setParentItem(nullptr);
        }
        else if(qobject_cast<Frame*>(item)) {
            Frame *frame = qobject_cast<Frame*>(item);
            m_scene->deleteFrame(frame);
            frame->setParentItem(nullptr);
            for(auto item: frame->contentList()) {
                if(qobject_cast<Node*>(item)) {
                    Node *node = qobject_cast<Node*>(item);
                    node->setAttachedFrame(nullptr);
                }
            }
        }
    }
}

void DeleteCommand::deleteWithSaveConnection() {
    if(m_newEdges.empty()) {
        for(auto item: m_items) {
            if(qobject_cast<Node*>(item)) {
                Node *node = qobject_cast<Node*>(item);
                Socket *outputSocket = node->getOutputSocket(0);
                if(outputSocket) {
                    //find chains of selected nodes
                    for(auto edge: outputSocket->getEdges()) {
                        Node *outNode = qobject_cast<Node*>(edge->endSocket()->parentItem());
                        if(outNode && outNode->selected()) continue;
                        Socket *inputSocket = node->getInputSocket(0);
                        Node *inNode = nullptr;
                        if(inputSocket && inputSocket->countEdge() > 0) inNode = qobject_cast<Node*>(inputSocket->getEdges()[0]->startSocket()->parentItem());
                        while(inNode && inNode->selected()) {
                            inputSocket = inNode->getInputSocket(0);
                            if(inputSocket && inputSocket->countEdge() > 0) inNode = qobject_cast<Node*>(inputSocket->getEdges()[0]->startSocket()->parentItem());
                        }
                        if(outNode && inNode) {
                            Edge *newEdge = new Edge(m_scene);
                            newEdge->setEndSocket(edge->endSocket());
                            newEdge->setStartSocket(inNode->getOutputSocket(0));
                            newEdge->endSocket()->setValue(newEdge->startSocket()->value());
                            m_newEdges.push_back(newEdge);
                        }
                    }
                }
            }
        }
    }
    deleteBase();
    for(auto edge: m_newEdges) {
        edge->setEndPosition(edge->endSocket()->globalPos());
        edge->setStartPosition(edge->startSocket()->globalPos());
        edge->endSocket()->addEdge(edge);
        edge->startSocket()->addEdge(edge);
        m_scene->addEdge(edge);
        edge->setParentItem(m_scene);
        edge->endSocket()->setValue(edge->startSocket()->value());
    }
}

void DeleteCommand::cancelDelete() {
    m_scene->clearSelected();
    for(auto item: m_items) {
        if(qobject_cast<Node*>(item)) {
            Node *node = qobject_cast<Node*>(item);
            m_scene->addNode(node);
            if(node->attachedFrame()) node->attachedFrame()->addNodes(QList<QQuickItem*>({node}));
            node->setParentItem(m_scene);
            m_scene->addSelected(node);
            node->generatePreview();
        }
        else if(qobject_cast<Edge*>(item)) {
            Edge *edge = qobject_cast<Edge*>(item);
            if(m_saveConnection) {
                for(auto edge: m_newEdges) {
                    edge->endSocket()->deleteEdge(edge);
                    edge->startSocket()->deleteEdge(edge);
                    m_scene->deleteEdge(edge);
                    edge->setParentItem(nullptr);
                }
            }
            m_scene->addEdge(edge);
            edge->startSocket()->addEdge(edge);
            edge->endSocket()->addEdge(edge);
            edge->setStartPosition(edge->startSocket()->globalPos());
            edge->setEndPosition(edge->endSocket()->globalPos());
            edge->setParentItem(m_scene);
            edge->endSocket()->setValue(edge->startSocket()->value());
        }
        else if(qobject_cast<Frame*>(item)) {
            Frame *frame = qobject_cast<Frame*>(item);
            m_scene->addFrame(frame);
            frame->setParentItem(m_scene);
            m_scene->addSelected(frame);
            for(auto item: frame->contentList()) {
                if(qobject_cast<Node*>(item)) {
                    Node *node = qobject_cast<Node*>(item);
                    node->setAttachedFrame(frame);
                }
            }
        }
    }
}

SelectCommand::SelectCommand(QList<QQuickItem*> items, QList<QQuickItem*> oldSelected, Scene *scene, QUndoCommand *parent): QUndoCommand (parent),
    m_scene(scene), m_items(items), m_oldSelected(oldSelected){
}

SelectCommand::~SelectCommand() {
    m_items.clear();
    m_oldSelected.clear();
    m_scene = nullptr;
}

void SelectCommand::undo() {
    m_scene->clearSelected();
    for(auto item: m_items) {
        if(qobject_cast<Node*>(item)) {
            Node *node = qobject_cast<Node*>(item);
            node->setSelected(false);
            m_scene->deleteSelected(node);
        }
        else if(qobject_cast<Frame*>(item)) {
            Frame *frame = qobject_cast<Frame*>(item);
            frame->setSelected(false);
            m_scene->deleteSelected(frame);
        }
        else if(qobject_cast<Edge*>(item)) {
            Edge *edge = qobject_cast<Edge*>(item);
            edge->setSelected(false);
            m_scene->deleteSelected(edge);
        }
    }
    for(auto item: m_oldSelected) {
        if(qobject_cast<Node*>(item)) {
            Node *node = qobject_cast<Node*>(item);
            node->setSelected(true);
            m_scene->addSelected(node);
        }
        else if(qobject_cast<Frame*>(item)) {
            Frame *frame = qobject_cast<Frame*>(item);
            frame->setSelected(true);
            m_scene->addSelected(frame);
        }
        else if(qobject_cast<Edge*>(item)) {
            Edge *edge = qobject_cast<Edge*>(item);
            edge->setSelected(true);
            m_scene->addSelected(edge);
        }
    }
}

void SelectCommand::redo() {
    m_scene->clearSelected();
    for(auto item: m_oldSelected) {
        if(qobject_cast<Node*>(item)) {
            Node *node = qobject_cast<Node*>(item);
            node->setSelected(false);
            m_scene->deleteSelected(node);
        }
        else if(qobject_cast<Frame*>(item)) {
            Frame *frame = qobject_cast<Frame*>(item);
            frame->setSelected(false);
            m_scene->deleteSelected(frame);
        }
        else if(qobject_cast<Edge*>(item)) {
            Edge *edge = qobject_cast<Edge*>(item);
            edge->setSelected(false);
            m_scene->deleteSelected(edge);
        }
    }
    for(auto item: m_items) {
        if(qobject_cast<Node*>(item)) {
            Node *node = qobject_cast<Node*>(item);
            node->setSelected(true);
            m_scene->addSelected(node);
        }
        else if (qobject_cast<Frame*>(item)) {
            Frame *frame = qobject_cast<Frame*>(item);
            frame->setSelected(true);
            m_scene->addSelected(frame);
        }
        else if(qobject_cast<Edge*>(item)) {
            Edge *edge = qobject_cast<Edge*>(item);
            edge->setSelected(true);
            m_scene->addSelected(edge);
        }
    }
}

PasteCommand::PasteCommand(QList<QQuickItem*> items, Scene *scene, QUndoCommand *parent):QUndoCommand (parent),
    m_scene(scene), m_pastedItems(items){
}

PasteCommand::~PasteCommand(){
    m_pastedItems.clear();
    m_scene = nullptr;
}

void PasteCommand::undo() {
    for(auto item: m_pastedItems) {
        if(qobject_cast<Node*>(item)) {
            Node *node = qobject_cast<Node*>(item);
            node->setParentItem(nullptr);
            m_scene->deleteNode(node);
        }
        else if(qobject_cast<Edge*>(item)) {
            Edge *edge = qobject_cast<Edge*>(item);
            edge->setParentItem(nullptr);
            m_scene->deleteEdge(edge);
        }
        else if(qobject_cast<Frame*>(item)) {
            Frame *frame = qobject_cast<Frame*>(item);
            frame->setParentItem(nullptr);
            m_scene->deleteFrame(frame);
        }
    }
    m_scene->clearSelected();
}

void PasteCommand::redo() {
    m_scene->clearSelected();
    for(auto item: m_pastedItems) {
        if(qobject_cast<Node*>(item)) {
            Node *node = qobject_cast<Node*>(item);
            node->setParentItem(m_scene);
            node->setSelected(true);
            m_scene->addNode(node);
            m_scene->addSelected(node);
            node->generatePreview();
        }
        else if(qobject_cast<Edge*>(item)) {
            Edge *edge = qobject_cast<Edge*>(item);
            edge->setParentItem(m_scene);
            m_scene->addEdge(edge);
        }
        else if(qobject_cast<Frame*>(item)) {
            Frame *frame = qobject_cast<Frame*>(item);
            frame->setParentItem(m_scene);
            frame->setSelected(true);
            m_scene->addFrame(frame);
            m_scene->addSelected(frame);
        }
    }
}

PropertyChangeCommand::PropertyChangeCommand(QQuickItem *item, const char* propName, QVariant newValue,
                                             QVariant oldValue, QUndoCommand *parent):
    QUndoCommand (parent), m_item(item), m_propName(propName), m_oldValue(oldValue), m_newValue(newValue) {

}

PropertyChangeCommand::~PropertyChangeCommand() {
    m_item = nullptr;
    delete [] m_propName;
}

void PropertyChangeCommand::undo() {
    if(qobject_cast<Node*>(m_item)) {
        Node *n = qobject_cast<Node*>(m_item);
        n->getPropertyPanel()->setProperty(m_propName, m_oldValue);
    }
    else if(qobject_cast<Frame*>(m_item)) {
        Frame *f = qobject_cast<Frame*>(m_item);
        f->getPropertyPanel()->setProperty(m_propName, m_oldValue);
    }
}

void PropertyChangeCommand::redo() {
    if(qobject_cast<Node*>(m_item)) {
        Node *n = qobject_cast<Node*>(m_item);
        n->getPropertyPanel()->setProperty(m_propName, m_newValue);
    }
    else if(qobject_cast<Frame*>(m_item)) {
        Frame *f = qobject_cast<Frame*>(m_item);
        f->getPropertyPanel()->setProperty(m_propName, m_newValue);
    }
}

MoveEdgeCommand::MoveEdgeCommand(Edge *edge, Socket *oldEndSocket, Socket *newEndSocket,
                                 QUndoCommand *parent): QUndoCommand (parent), m_edge(edge),
    m_oldSocket(oldEndSocket), m_newSocket(newEndSocket){

}

MoveEdgeCommand::~MoveEdgeCommand() {
    m_edge = nullptr;
    m_oldSocket = nullptr;
    m_newSocket = nullptr;
}

void MoveEdgeCommand::undo() {
    m_newSocket->deleteEdge(m_edge);
    m_edge->setEndSocket(m_oldSocket);
    m_oldSocket->addEdge(m_edge);
    m_edge->setEndPosition(m_oldSocket->globalPos());
    m_oldSocket->setValue(m_edge->startSocket()->value());
}

void MoveEdgeCommand::redo() {
    m_oldSocket->deleteEdge(m_edge);
    m_edge->setEndSocket(m_newSocket);
    m_newSocket->addEdge(m_edge);
    m_edge->setEndPosition(m_newSocket->globalPos());
    m_newSocket->setValue(m_edge->startSocket()->value());
}

DetachFromFrameCommand::DetachFromFrameCommand(QList<QPair<QQuickItem *, Frame *> > data, QUndoCommand *parent):
    QUndoCommand (parent), m_data(data){

}

DetachFromFrameCommand::~DetachFromFrameCommand() {
    m_data.clear();
}

void DetachFromFrameCommand::undo() {
    for(auto pair: m_data) {
        pair.second->addNodes(QList<QQuickItem*>({pair.first}));
    }
}

void DetachFromFrameCommand::redo() {
    for(auto pair: m_data) {
        pair.second->removeItem(pair.first);
        if(qobject_cast<Node*>(pair.first)) {
            Node *node = qobject_cast<Node*>(pair.first);
            node->setAttachedFrame(nullptr);
        }
    }
}

ResizeFrameCommand::ResizeFrameCommand(Frame *frame, float offsetX, float offsetY, float offsetWidth,
                                       float offsetHeight): m_frame(frame), m_offsetX(offsetX),
    m_offsetY(offsetY), m_offsetWidth(offsetWidth), m_offsetHeight(offsetHeight) {

}

ResizeFrameCommand::~ResizeFrameCommand() {
    m_frame = nullptr;
}

void ResizeFrameCommand::undo() {
    m_frame->setBaseX(m_frame->baseX() - m_offsetX);
    m_frame->setBaseY(m_frame->baseY() - m_offsetY);
    m_frame->setWidth(m_frame->width() - m_offsetWidth);
    m_frame->setHeight(m_frame->height() - m_offsetHeight);
}

void ResizeFrameCommand::redo() {
    m_frame->setBaseX(m_frame->baseX() + m_offsetX);
    m_frame->setBaseY(m_frame->baseY() + m_offsetY);
    m_frame->setWidth(m_frame->width() + m_offsetWidth);
    m_frame->setHeight(m_frame->height() + m_offsetHeight);
}

bool ResizeFrameCommand::mergeWith(const QUndoCommand *command) {
    if(id() == command->id()) {
        const ResizeFrameCommand *other = static_cast<const ResizeFrameCommand*>(command);
        if(m_frame != other->m_frame) return false;
        m_offsetX += other->m_offsetX;
        m_offsetY += other->m_offsetY;
        m_offsetWidth += other->m_offsetWidth;
        m_offsetHeight += other->m_offsetHeight;
        return true;
    }
    return false;
}

int ResizeFrameCommand::id() const {
    return 1;
}

ChangeTitleCommand::ChangeTitleCommand(Frame *frame, QString newTitle, QString oldTitle): m_frame(frame),
    m_newTitle(newTitle), m_oldTitle(oldTitle) {

}

ChangeTitleCommand::~ChangeTitleCommand() {
    m_frame = nullptr;
}

void ChangeTitleCommand::undo() {
    m_frame->setTitle(m_oldTitle);
}

void ChangeTitleCommand::redo() {
    m_frame->setTitle(m_newTitle);
}

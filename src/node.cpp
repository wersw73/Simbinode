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

#include "node.h"
#include "scene.h"
#include <iostream>
#include <QQmlProperty>

Node::Node(QQuickItem *parent, QVector2D resolution, GLint bpc): QQuickItem (parent), m_resolution(resolution), m_bpc(bpc)
{
    setFlag(ItemHasContents, true);
    setAcceptedMouseButtons(Qt::AllButtons);
    setAcceptHoverEvents(true);
    setTransformOrigin(TopLeft);
    view = new QQuickView();
    view->setSource(QUrl(QStringLiteral("qrc:/qml/Node.qml")));
    grNode = qobject_cast<QQuickItem *>(view->rootObject());
    grNode->setParentItem(this);
    grNode->setX(8);
    setZ(4);
    setWidth(196);
    setHeight(207);
    grNode->setHeight(207);
}

Node::Node(const Node &node):Node() {
    setBaseX(node.m_baseX);
    setBaseY(node.m_baseY);
    createSockets(node.m_socketsInput.count(), node.m_socketOutput.count());
    createAdditionalInputs(node.m_additionalInputs.count());
}

Node::~Node() {
    for(auto s: m_socketsInput) {
        delete s;
    }

    for(auto s: m_socketOutput) {
        delete s;
    }

    for(auto s: m_additionalInputs) {
        delete s;
    }

    delete grNode;
    delete view;
    delete propertiesPanel;
    delete propView;
}

float Node::baseX() {
    return m_baseX;
}

void Node::setBaseX(float value) {
    m_baseX = value;
    setX(m_baseX*m_scale - m_pan.x());
    for(auto s: m_socketsInput) {
        QPointF sPos = mapToItem(parentItem(), QPointF(s->x() + 8, s->y() + 8));
        s->setGlobalPos(QVector2D(sPos.x(), sPos.y()));
    }
    for(auto s: m_socketOutput) {
        QPointF sPos = mapToItem(parentItem(), QPointF(s->x() + 8, s->y() + 8));
        s->setGlobalPos(QVector2D(sPos.x(), sPos.y()));
    }
    for(auto s: m_additionalInputs) {
        QPointF sPos = mapToItem(parentItem(), QPointF(s->x() + 8, s->y() + 8));
        s->setGlobalPos(QVector2D(sPos.x(), sPos.y()));
    }
    if(m_attachedFrame && !m_attachedFrame->selected()) {
        m_attachedFrame->resizeByContent();
    }
    emit changeBaseX(value);
}

float Node::baseY() {
    return m_baseY;
}

void Node::setBaseY(float value) {
    m_baseY = value;
    setY(m_baseY*m_scale - m_pan.y());
    for(auto s: m_socketsInput) {
        QPointF sPos = mapToItem(parentItem(), QPointF(s->x() + 8, s->y() + 8));
        s->setGlobalPos(QVector2D(sPos.x(), sPos.y()));
    }
    for(auto s: m_socketOutput) {
        QPointF sPos = mapToItem(parentItem(), QPointF(s->x() + 8, s->y() + 8));
        s->setGlobalPos(QVector2D(sPos.x(), sPos.y()));
    }
    for(auto s: m_additionalInputs) {
        QPointF sPos = mapToItem(parentItem(), QPointF(s->x() + 8, s->y() + 8));
        s->setGlobalPos(QVector2D(sPos.x(), sPos.y()));
    }
    if(m_attachedFrame && !m_attachedFrame->selected()) {
        m_attachedFrame->resizeByContent();
    }
    emit changeBaseY(value);
}

QVector2D Node::pan() {
    return m_pan;
}

void Node::setPan(QVector2D pan) {
    m_pan = pan;
    setX(m_baseX*m_scale - m_pan.x());
    setY(m_baseY*m_scale - m_pan.y());
    for(auto s: m_socketsInput) {
        QPointF sPos = mapToItem(parentItem(), QPointF(s->x() + 8, s->y() + 8));
        s->setGlobalPos(QVector2D(sPos.x(), sPos.y()));
    }
    for(auto s: m_socketOutput) {
        QPointF sPos = mapToItem(parentItem(), QPointF(s->x() + 8, s->y() + 8));
        s->setGlobalPos(QVector2D(sPos.x(), sPos.y()));
    }
    for(auto s: m_additionalInputs) {
        QPointF sPos = mapToItem(parentItem(), QPointF(s->x() + 8, s->y() + 8));
        s->setGlobalPos(QVector2D(sPos.x(), sPos.y()));
    }
    emit changePan(pan);
}

QVector2D Node::resolution() {
    return m_resolution;
}

void Node::setResolution(QVector2D res) {
    m_resolution = res;
    emit changeResolution(res);
}

GLint Node::bpc() {
    return m_bpc;
}

void Node::setBPC(GLint bpc) {
    if(m_bpc == bpc) return;
    m_bpc = bpc;
    emit changeBPC(bpc);
}

float Node::scaleView(){
    return m_scale;
}

bool Node::selected() {
    return m_selected;
}

void Node::setSelected(bool select) {
    m_selected = select;
    grNode->setProperty("selected", select);
    emit changeSelected(select);
}

bool Node::checkConnected(Node *node, socketType type) {
    if(type == OUTPUTS) {
        for(auto socket: m_socketsInput) {
            for(auto edge: socket->getEdges()) {
                if(qobject_cast<Node*>(edge->startSocket()->parentItem())) {
                    Node *startNode = qobject_cast<Node*>(edge->startSocket()->parentItem());
                    if(startNode == node) {
                        return true;
                    }
                    else {
                        bool connected = startNode->checkConnected(node, type);
                        if(connected) return true;
                    }
                }
            }
        }
        for(auto socket: m_additionalInputs) {
            for(auto edge: socket->getEdges()) {
                if(qobject_cast<Node*>(edge->startSocket()->parentItem())) {
                    Node *startNode = qobject_cast<Node*>(edge->startSocket()->parentItem());
                    if(startNode == node) {
                        return true;
                    }
                    else {
                        bool connected = startNode->checkConnected(node, type);
                        if(connected) return true;
                    }
                }
            }
        }
        return false;
    }
    else {
        for(auto socket: m_socketOutput) {
            for(auto edge: socket->getEdges()) {
                if(qobject_cast<Node*>(edge->endSocket()->parentItem())) {
                    Node *endNode = qobject_cast<Node*>(edge->endSocket()->parentItem());
                    if(endNode == node) {
                        return true;
                    }
                    else {
                        bool connected = endNode->checkConnected(node, type);
                        if(connected) return true;
                    }
                }
            }
        }        
        return false;
    }
}

bool Node::isPointInRadius(QVector2D point) {
    return (x() - 30 < point.x() && x() + width()*scale() + 30 > point.x() && y() - 30 < point.y() && y() + height()*scale() + 30 > point.y());
}

Socket *Node::getNearestOutputSocket(QVector2D center, float radius) {
    Socket *nearestSocket = nullptr;
    float minDist = std::numeric_limits<float>::max();
    for(auto socket: m_socketOutput) {
        bool inRadius = socket->inCircle(center, radius);
        if(inRadius) {
            float dist = (socket->globalPos() - center).length();
            if(dist < minDist) {
                minDist = dist;
                nearestSocket = socket;
            }
        }
    }
    return nearestSocket;
}

Socket *Node::getNearestInputSocket(QVector2D center, float radius) {
    Socket *nearestSocket = nullptr;
    float minDist = std::numeric_limits<float>::max();
    for(auto socket: m_socketsInput) {
        bool inRadius = socket->inCircle(center, radius);
        if(inRadius) {
            float dist = (socket->globalPos() - center).length();
            if(dist < minDist) {
                minDist = dist;
                nearestSocket = socket;
            }
        }
    }
    for(auto socket: m_additionalInputs) {
        bool inRadius = socket->inCircle(center, radius);
        if(inRadius) {
            float dist = (socket->globalPos() - center).length();
            if(dist < minDist) {
                minDist = dist;
                nearestSocket = socket;
            }
        }
    }
    return nearestSocket;
}

Socket *Node::getInputSocket(int index) const {
    if(index < m_socketsInput.count() + m_additionalInputs.count()) {
        if(index < m_socketsInput.count()) return m_socketsInput[index];
        else return m_additionalInputs[index - m_socketsInput.count()];
    }
    return nullptr;
}

Socket *Node::getOutputSocket(int index) const {
    if(index < m_socketOutput.count()) return m_socketOutput[index];
    return nullptr;
}

QList<Edge*> Node::getEdges() const {
    QList<Edge*> edges;
    for(auto s: m_socketsInput) {
        edges.append(s->getEdges());
    }
    for(auto s: m_socketOutput) {
        edges.append(s->getEdges());
    }
    for(auto s: m_additionalInputs) {
        edges.append(s->getEdges());
    }
    return edges;
}

QQuickItem *Node::getPropertyPanel() {
    return propertiesPanel;
}

Frame *Node::attachedFrame() {
    return m_attachedFrame;
}

void Node::setAttachedFrame(Frame *frame) {
    m_attachedFrame = frame;
}

void Node::mousePressEvent(QMouseEvent *event) {
    if(event->pos().x() < 8 || event->pos().x() > 186 || event->pos().y() > 207) {
        event->setAccepted(false);
        return;
    }
    setFocus(true);
    setFocus(false);
    moved = false;
    m_intersectingEdge = nullptr;
    Scene *scene = reinterpret_cast<Scene*>(parentItem());
    QPointF point = mapToItem(scene, QPointF(event->pos().x(), event->pos().y())); 
    dragX = event->pos().x()*scale();
    dragY = event->pos().y()*scale();
    oldX = m_baseX;
    oldY = m_baseY;
    if(event->button() == Qt::LeftButton && event->modifiers() == Qt::ControlModifier) {
        setSelected(!m_selected);
        if(m_selected) {
            QList<QQuickItem*> selected = scene->selectedList();
            scene->addSelected(this);
            scene->selectedItems(selected);
        }
        else {
            QList<QQuickItem*> selected = scene->selectedList();
            scene->deleteSelected(this);
            scene->selectedItems(selected);
        }

    }
    else if(event->button() == Qt::LeftButton && !m_selected) {
        QList<QQuickItem*> selected = scene->selectedList();
        scene->clearSelected();
        setSelected(true);
        scene->addSelected(this);        
        scene->selectedItems(selected);
    }
    else if(event->button() != Qt::LeftButton) {
        event->setAccepted(false);
    }
}

void Node::mouseMoveEvent(QMouseEvent *event) {
    if(event->buttons() == Qt::LeftButton && event->modifiers() == Qt::NoModifier) {
        if(!moved) {
            moved = true;
        }

        Scene* scene = reinterpret_cast<Scene*>(parentItem());
        QPointF point = mapToItem(scene, QPointF(event->pos().x(), event->pos().y()));
        setX(point.x() - dragX);
        setY(point.y() - dragY);
        int offsetBaseX = m_baseX - (x() + m_pan.x())/m_scale;
        int offsetBaseY = m_baseY - (y() + m_pan.y())/m_scale;
        for(int i = 0; i < scene->countSelected(); ++i) {
            QQuickItem *item = scene->atSelected(i);
            if(qobject_cast<Node*>(item)) {
                Node *n = qobject_cast<Node*>(item);
                n->setBaseX(n->baseX() - offsetBaseX);
                n->setBaseY(n->baseY() - offsetBaseY);
            }
            else if(qobject_cast<Frame*>(item)) {
                Frame *f = qobject_cast<Frame*>(item);
                f->setBaseX(f->baseX() - offsetBaseX);
                f->setBaseY(f->baseY() - offsetBaseY);
            }
        }

        if(scene->countSelected() == 1 && getEdges().count() == 0 && !m_socketsInput.empty() && !m_socketOutput.empty()) {
            Edge *edge = nullptr;
            for(auto e: scene->edges()) {
                bool intersecting = e->intersectWith(x(), y(), width()*m_scale, height()*m_scale);
                if(intersecting) {
                    edge = e;
                    break;
                }
            }
            if(edge) {
                if(m_intersectingEdge != edge) {
                    if(m_intersectingEdge) m_intersectingEdge->setSelected(false);
                    edge->setSelected(true);
                }
            }
            else {
                 if(m_intersectingEdge) m_intersectingEdge->setSelected(false);
            }
            m_intersectingEdge = edge;
        }
    }
}

void Node::mouseReleaseEvent(QMouseEvent *event) {
    if(event->button() == Qt::LeftButton && event->modifiers() != Qt::ControlModifier) {
        QVector2D offset(m_baseX - oldX, m_baseY - oldY);

        Scene* scene = reinterpret_cast<Scene*>(parentItem());
        if(!moved && m_selected){
            QList<QQuickItem*> selected = scene->selectedList();
            if(selected.size() > 1) {
                scene->clearSelected();
                setSelected(true);
                scene->addSelected(this);
                scene->selectedItems(selected);
            }
        }
        else {
            QPointF scenePoint = mapToItem(scene, QPointF(width()*0.5f, height()*0.16f));
            Frame *frame = scene->frameAt(scenePoint.x(), scenePoint.y());
            if(m_intersectingEdge) m_intersectingEdge->setSelected(false);
            frame = attachedFrame() ? nullptr : frame;
            scene->movedNodes(scene->selectedList(), offset, frame, m_intersectingEdge);
        }
    }
}

void Node::hoverMoveEvent(QHoverEvent *event) {
    if(!grNode->property("hovered").toBool() && event->pos().y() < 207) {
        grNode->setProperty("hovered", true);
        if(m_attachedFrame) {
            m_attachedFrame->setBubbleVisible(true);
        }
    }
    else if(grNode->property("hovered").toBool() && event->pos().y() > 207) {
        grNode->setProperty("hovered", false);
        if(m_attachedFrame) {
            m_attachedFrame->setBubbleVisible(false);
        }
    }
}

void Node::hoverLeaveEvent(QHoverEvent *event) {
    grNode->setProperty("hovered", false);
    if(m_attachedFrame) {
        m_attachedFrame->setBubbleVisible(false);
    }
}

Node *Node::clone() {
    return new Node(parentItem(), m_resolution, m_bpc);
}

void Node::serialize(QJsonObject &json) const {
    json["name"] = objectName();
    json["baseX"] = m_baseX;
    json["baseY"] = m_baseY;
    json["bpc"] = m_bpc;
    QJsonArray inputs;
    for(Socket *s: m_socketsInput) {
        QJsonObject socketObject;
        s->serialize(socketObject);
        inputs.push_back(socketObject);
    }
    json["inputs"] = inputs;
    QJsonArray outputs;
    for(Socket *s: m_socketOutput) {
        QJsonObject socketObject;
        s->serialize(socketObject);
        outputs.push_back(socketObject);
    }
    json["outputs"] = outputs;
    QJsonArray additionals;
    for(Socket *s: m_additionalInputs) {
        QJsonObject socketObject;
        s->serialize(socketObject);
        additionals.push_back(socketObject);
    }
    json["additionals"] = additionals;
}

void Node::deserialize(const QJsonObject &json, QHash<QUuid, Socket *> &hash) {
    deserializing = true;
    if(json.contains("baseX")) {        
        setBaseX(json["baseX"].toVariant().toFloat());
    }
    if(json.contains("baseY")) {
        setBaseY(json["baseY"].toVariant().toFloat());
    }
    if(json.contains("bpc")) {
        setBPC(json["bpc"].toInt());
    }
    if(json.contains("inputs")) {
        QJsonArray inputs = json["inputs"].toArray();
        for(int i = 0; i < inputs.size(); ++i) {
            QJsonObject inputObject = inputs[i].toObject();
            Socket *s = m_socketsInput[i];
            s->deserialize(inputObject);
            hash[s->id()] = s;
        }
    }
    if(json.contains("outputs")) {
        QJsonArray outputs = json["outputs"].toArray();
        for(int i = 0; i < outputs.size(); ++i) {
            QJsonObject outputObject = outputs[i].toObject();
            Socket *s = m_socketOutput[i];
            s->deserialize(outputObject);
            hash[s->id()] = s;
        }
    }
    if(json.contains("additionals")) {
        QJsonArray additionals = json["additionals"].toArray();
        for(int i = 0; i < additionals.size(); ++i) {
            QJsonObject additionalsObject = additionals[i].toObject();
            Socket *s = m_additionalInputs[i] ;
            s->deserialize(additionalsObject);
            hash[s->id()] = s;
        }
    }
}

void Node::createSockets(int inputCount, int outputCount) {
    m_socketsInput.clear();
    m_socketOutput.clear();
    float inputStart = (grNode->height() - 30*m_scale)/2 + 22*m_scale - 42*m_scale*(inputCount - 1)/2;
    float outputStart = (grNode->height() - 30*m_scale)/2 + 22*m_scale - 42*m_scale*(outputCount - 1)/2;
    for(int i = 0; i < inputCount; ++i) {
        Socket *s = new Socket(this);
        s->setType(INPUTS);
        s->setY(inputStart + 42*m_scale*i);
        s->setX(2*m_scale);
        s->updateScale(m_scale);
        QPointF sPos = mapToItem(parentItem(), QPointF(s->x() + 8*m_scale, s->y() + 8*m_scale));
        s->setGlobalPos(QVector2D(sPos.x(), sPos.y()));
        m_socketsInput.append(s);
    }
    for(int i = 0; i < outputCount; ++i) {
        Socket *s = new Socket(this);
        s->setType(OUTPUTS);
        s->setTip("Output");
        s->setY(outputStart + 42*m_scale*i);
        s->setX(178*m_scale);
        s->updateScale(m_scale);
        QPointF sPos = mapToItem(parentItem(), QPointF(s->x() + 8, s->y() + 8));
        s->setGlobalPos(QVector2D(sPos.x(), sPos.y()));
        m_socketOutput.append(s);
    }
}

void Node::createAdditionalInputs(int count) {
    m_additionalInputs.clear();
    float start = grNode->height() + 12*m_scale;
    for(int i = 0; i < count; ++i) {
        Socket *s = new Socket(this);
        s->setType(INPUTS);
        s->setAdditional(true);
        s->setY(start + 28*m_scale*i);
        s->setX(2*m_scale);
        s->updateScale(m_scale);
        QPointF sPos = mapToItem(parentItem(), QPointF(s->x() + 8*m_scale, s->y() + 8*m_scale));
        s->setGlobalPos(QVector2D(sPos.x(), sPos.y()));
        m_additionalInputs.append(s);
    }
}

void Node::setTitle(QString title) {
    grNode->setProperty("title", title);
}

void Node::setPropertyOnPanel(const char *name, QVariant value) {
    propertiesPanel->setProperty(name, value);
}

void Node::propertyChanged(QString propName, QVariant newValue, QVariant oldValue) {
    Scene* scene = reinterpret_cast<Scene*>(parentItem());
    if(scene) {
        std::string prop = propName.toStdString();
        char *name = new char[prop.size() + 1];
        std::copy(prop.begin(), prop.end(), name);
        name[prop.size()] = '\0';
        scene->itemPropertyChanged(this, name, newValue, oldValue);
    }
}

void Node::operation() {

}

unsigned int &Node::getPreviewTexture() {
    return previewTex;
}

void Node::saveTexture(QString fileName) {

}

void Node::scaleUpdate(float scale) {
    setScale(scale);
    grNode->setProperty("scaleView", scale);
    int inputCount = m_socketsInput.length();
    int outputCount = m_socketOutput.length();
    for(int i = 0; i < inputCount; ++i) {
        Socket *s = m_socketsInput[i];
        s->updateScale(scale);
        QPointF sPos = mapToItem(parentItem(), QPointF(s->x() + 8, s->y() + 8));
        s->setGlobalPos(QVector2D(sPos.x(), sPos.y()));
    }
    for(int i = 0; i < outputCount; ++i) {
        Socket *s = m_socketOutput[i];
        s->updateScale(scale);
        QPointF sPos = mapToItem(parentItem(), QPointF(s->x() + 8, s->y() + 8));
        s->setGlobalPos(QVector2D(sPos.x(), sPos.y()));
    }
    for(int i = 0; i < m_additionalInputs.length(); ++i) {
        Socket *s = m_additionalInputs[i];
        s->updateScale(scale);
        QPointF sPos = mapToItem(parentItem(), QPointF(s->x() + 8, s->y() + 8));
        s->setGlobalPos(QVector2D(sPos.x(), sPos.y()));
    }
    m_scale = scale;
    changeScaleView(scale);
}

void Node::bpcUpdate(int bpcType) {
    switch (bpcType) {
        case 0:
        default:
            setBPC(GL_RGBA8);
            break;
        case 1:
            setBPC(GL_RGBA16);
            break;
    }
}

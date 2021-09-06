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

#include "roughnode.h"

RoughNode::RoughNode(QQuickItem *parent, QVector2D resolution, GLint bpc): Node(parent, resolution, bpc)
{
    createSockets(1, 0);
    setTitle("Roughness");
    m_socketsInput[0]->setTip("Roughness");
    preview = new OneChanelObject(grNode, m_resolution, m_bpc);
    float s = scaleView();
    preview->setTransformOrigin(TopLeft);
    preview->setWidth(174);
    preview->setHeight(174);
    preview->setX(3*s);
    preview->setY(30*s);
    preview->setScale(s);
    preview->setValue(0.2f);
    connect(preview, &OneChanelObject::updatePreview, this, &RoughNode::updatePreview);
    connect(preview, &OneChanelObject::updateValue, this, &RoughNode::roughChanged);
    connect(this, &Node::changeResolution, preview, &OneChanelObject::setResolution);
    connect(this, &Node::changeBPC, preview, &OneChanelObject::setBPC);
    propView = new QQuickView();
    propView->setSource(QUrl(QStringLiteral("qrc:/qml/RoughProperty.qml")));
    propertiesPanel = qobject_cast<QQuickItem*>(propView->rootObject());
    if(m_bpc == GL_RGBA8) propertiesPanel->setProperty("startBits", 0);
    else if(m_bpc == GL_RGBA16) propertiesPanel->setProperty("startBits", 1);
    connect(propertiesPanel, SIGNAL(roughChanged(qreal)), this, SLOT(updateRough(qreal)));
    connect(propertiesPanel, SIGNAL(bitsChanged(int)), this, SLOT(bpcUpdate(int)));
    connect(propertiesPanel, SIGNAL(propertyChangingFinished(QString, QVariant, QVariant)), this, SLOT(propertyChanged(QString, QVariant, QVariant)));
}

RoughNode::~RoughNode() {
    delete preview;
}

void RoughNode::operation() {
    if(!m_socketsInput[0]->getEdges().isEmpty()) {
        Node *inputNode0 = static_cast<Node*>(m_socketsInput[0]->getEdges()[0]->startSocket()->parentItem());
        if(inputNode0 && inputNode0->resolution() != m_resolution) return;
        if(m_socketsInput[0]->value() == 0 && deserializing) return;
        preview->useTex = true;
        preview->setValue(m_socketsInput[0]->value().toUInt());
    }
    else {
        preview->useTex = false;
        preview->setValue(m_rough);
    }
    preview->selectedItem = selected();
    preview->update();
    if(deserializing) deserializing = false;
}

unsigned int &RoughNode::getPreviewTexture() {
    return preview->texture();
}

void RoughNode::saveTexture(QString fileName) {
    preview->saveTexture(fileName);
}

void RoughNode::serialize(QJsonObject &json) const {
    Node::serialize(json);
    json["type"] = 10;
    json["rough"] = m_rough;
}

void RoughNode::deserialize(const QJsonObject &json, QHash<QUuid, Socket*> &hash) {
    Node::deserialize(json, hash);
    if(json.contains("rough")) {
        propertiesPanel->setProperty("startRough", m_rough);
    }
    if(m_bpc == GL_RGBA8) propertiesPanel->setProperty("startBits", 0);
    else if(m_bpc == GL_RGBA16) propertiesPanel->setProperty("startBits", 1);
}

void RoughNode::updateRough(qreal rough) {
    if(m_rough == rough) return;
    m_rough = rough;
    operation();
    dataChanged();
}

void RoughNode::saveRough(QString dir) {
    preview->saveTexture(dir.append("/roughness.png"));
}

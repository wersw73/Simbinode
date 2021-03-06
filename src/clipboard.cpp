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

#include "clipboard.h"
#include "scene.h"
#include "noisenode.h"
#include "mixnode.h"
#include "normalmapnode.h"
#include "voronoinode.h"
#include "polygonnode.h"
#include "circlenode.h"
#include "transformnode.h"
#include "tilenode.h"
#include "warpnode.h"
#include "blurnode.h"
#include "inversenode.h"
#include "colorrampnode.h"
#include "colornode.h"
#include "coloringnode.h"
#include "mappingnode.h"
#include "mirrornode.h"
#include "brightnesscontrastnode.h"
#include "thresholdnode.h"
#include "albedonode.h"
#include "metalnode.h"
#include "roughnode.h"
#include "normalnode.h"
#include "heightnode.h"
#include "emissionnode.h"
#include "grayscalenode.h"
#include "gradientnode.h"
#include "directionalwarpnode.h"
#include "directionalblurnode.h"
#include "slopeblurnode.h"
#include "bevelnode.h"
#include "polartransformnode.h"
#include "bricksnode.h"
#include "hexagonsnode.h"
#include <iostream>

Clipboard::Clipboard()
{
}

Clipboard::~Clipboard() {
    for(auto node: clipboard_nodes) {
        if(node) delete node;
    }
    for(auto edge: clipboard_edges) {
        if(edge) delete edge;
    }
    for(auto frame: clipboard_frames) {
        if(frame) {
            for(auto item: frame->contentList()) {
                delete item;
            }
            delete frame;
        }
    }
}

void Clipboard::cut(Scene *scene) {
    copy(scene);
    scene->cut();
}

void Clipboard::copy(Scene *scene) {
    clear();
    QList<QQuickItem*> selected = scene->selectedList();
    QList<Frame*> sel_frames;
    QList<Node*> sel_nodes;
    QList<Edge*> sel_edges;
    float maxX = std::numeric_limits<float>::min();
    float maxY = maxX;
    float minX = std::numeric_limits<float>::max();
    float minY = minX;
    for(auto item: selected) {
        if(!(qobject_cast<AlbedoNode*>(item) || qobject_cast<MetalNode*>(item) || qobject_cast<RoughNode*>(item) || qobject_cast<NormalNode*>(item) || qobject_cast<HeightNode*>(item) || qobject_cast<EmissionNode*>(item))) {
            if(qobject_cast<Node*>(item)) {
                Node *node = qobject_cast<Node*>(item);
                if(!node->attachedFrame() || !node->attachedFrame()->selected()) sel_nodes.append(node);
                maxX = std::max(maxX, (float)(node->x() + node->width()*node->scale()));
                maxY = std::max(maxY, (float)(node->y() + node->height()*node->scale()));
                minX = std::min(minX, (float)node->x());
                minY = std::min(minY, (float)node->y());
                QList<Edge*> edges = node->getEdges();
                for(auto edge: edges) {
                    if(sel_edges.contains(edge)) continue;
                    Node *startNode = qobject_cast<Node*>(edge->startSocket()->parentItem());
                    Node *endNode = qobject_cast<Node*>(edge->endSocket()->parentItem());
                    if((startNode && startNode->selected()) && (!(qobject_cast<AlbedoNode*>(endNode) || qobject_cast<MetalNode*>(endNode) || qobject_cast<RoughNode*>(endNode) || qobject_cast<NormalNode*>(endNode) || qobject_cast<HeightNode*>(endNode) || qobject_cast<EmissionNode*>(endNode)) && endNode->selected())) {
                        sel_edges.append(edge);
                    }
                }
            }
            else if(qobject_cast<Frame*>(item)) {
                Frame *frame = qobject_cast<Frame*>(item);
                sel_frames.append(frame);
                maxX = std::max(maxX, (float)(frame->x() + frame->width()*frame->scale()));
                maxY = std::max(maxY, (float)(frame->y() + frame->height()*frame->scale()));
                minX = std::min(minX, (float)frame->x());
                minY = std::min(minY, (float)frame->y());
            }
        }
    }

    center.setX(((maxX - minX)*0.5 + minX + scene->background()->viewPan().x())/scene->background()->viewScale());
    center.setY(((maxY - minY)*0.5 + minY + scene->background()->viewPan().y())/scene->background()->viewScale());

    for(auto frame: sel_frames) {
        Frame *f = new Frame();
        f->setBaseX(frame->baseX());
        f->setBaseY(frame->baseY());
        f->setWidth(frame->width());
        f->setHeight(frame->height());
        f->setTitle(frame->title());
        f->setColor(frame->color());
        clipboard_frames.append(f);
        QList<QQuickItem*> copiedContent;
        for(auto item: frame->contentList()) {
            Node *baseNode = qobject_cast<Node*>(item);
            if(baseNode && !(qobject_cast<AlbedoNode*>(item) || qobject_cast<MetalNode*>(item) || qobject_cast<RoughNode*>(item) || qobject_cast<NormalNode*>(item) || qobject_cast<HeightNode*>(item) || qobject_cast<EmissionNode*>(item)) && baseNode->selected()) {
                Node *copiedContentNode = baseNode->clone();
                copiedContentNode->setParent(nullptr);
                copiedContentNode->setParentItem(nullptr);
                copiedContentNode->setBaseX(baseNode->baseX());
                copiedContentNode->setBaseY(baseNode->baseY());
                copiedContent.append(copiedContentNode);
            }
        }
        if(copiedContent.size() > 0) f->addNodes(copiedContent);
    }
    for(auto node: sel_nodes) {
        Node *copiedNode = node->clone();
        copiedNode->setParent(nullptr);
        copiedNode->setParentItem(nullptr);
        copiedNode->setBaseX(node->baseX());
        copiedNode->setBaseY(node->baseY());
        clipboard_nodes.append(copiedNode);
    }
    for(auto edge: sel_edges) {
        Edge *e = new Edge();
        e->setStartPosition((edge->startPosition() + scene->background()->viewPan())/scene->background()->viewScale());
        e->setEndPosition((edge->endPosition() + scene->background()->viewPan())/scene->background()->viewScale());
        clipboard_edges.append(e);
    }
}

void Clipboard::paste(float posX, float posY, Scene *scene) {
    QList<QQuickItem*> pastedItem;
    if(clipboard_nodes.empty() && clipboard_frames.empty()) return;
    std::cout << posX << " " << posY << std::endl;
    float viewScale = scene->background()->viewScale();
    QVector2D viewPan = scene->background()->viewPan();
    QVector2D currentCenter = center*viewScale - viewPan;

    scene->clearSelected();
    for(auto f: clipboard_frames) {
        Frame *frame = new Frame(scene);
        float x = posX - (currentCenter.x() - (f->baseX()*viewScale - viewPan.x()));
        float y = posY - (currentCenter.y() - (f->baseY()*viewScale - viewPan.y()));
        frame->setPan(viewPan);
        frame->setScaleView(viewScale);
        frame->setBaseX((viewPan.x() + x)/viewScale);
        frame->setBaseY((viewPan.y() + y)/viewScale);
        frame->setWidth(f->width());
        frame->setHeight(f->height());
        frame->setTitle(f->title());
        frame->setColor(f->color());
        frame->setSelected(true);
        scene->addFrame(frame);
        scene->addSelected(frame);
        pastedItem.append(frame);
        QList<QQuickItem*> pastedContent;
        for(auto item: f->contentList()) {
            if(qobject_cast<Node*>(item)) {
                Node *baseNode = qobject_cast<Node*>(item);
                Node *pastedNode = baseNode->clone();
                pastedNode->setParent(scene);
                pastedNode->setParentItem(scene);
                float x = posX - (currentCenter.x() - (baseNode->baseX()*viewScale - viewPan.x()));
                float y = posY - (currentCenter.y() - (baseNode->baseY()*viewScale - viewPan.y()));
                pastedNode->setBaseX((viewPan.x() + x)/viewScale);
                pastedNode->setBaseY((viewPan.y() + y)/viewScale);
                pastedNode->setSelected(true);
                scene->addNode(pastedNode);
                scene->addSelected(pastedNode);
                pastedContent.append(pastedNode);
                pastedItem.append(pastedNode);
            }
        }
        if(pastedContent.size() > 0) frame->addNodes(pastedContent);
    }
    for(auto n: clipboard_nodes) {
        Node *pastedNode = n->clone();
        pastedNode->setParent(scene);
        pastedNode->setParentItem(scene);
        float x = posX - (currentCenter.x() - (n->baseX()*viewScale - viewPan.x()));
        float y = posY - (currentCenter.y() - (n->baseY()*viewScale - viewPan.y()));
        pastedNode->setBaseX((viewPan.x() + x)/viewScale);
        pastedNode->setBaseY((viewPan.y() + y)/viewScale);
        pastedNode->setSelected(true);
        scene->addNode(pastedNode);
        scene->addSelected(pastedNode);
        pastedItem.append(pastedNode);
    }
    for(auto e: clipboard_edges) {
        Edge *edge = new Edge(scene);
        float startX = e->startPosition().x()*viewScale - viewPan.x();
        float startY = e->startPosition().y()*viewScale - viewPan.y();
        float endX = e->endPosition().x()*viewScale - viewPan.x();
        float endY = e->endPosition().y()*viewScale - viewPan.y();
        edge->setStartPosition(QVector2D(posX - (currentCenter.x() - startX), posY - (currentCenter.y() - startY)));
        edge->setEndPosition(QVector2D(posX - (currentCenter.x() - endX), posY - (currentCenter.y() - endY)));
        Socket *startSock = edge->findSockets(scene, edge->startPosition().x(), edge->startPosition().y());
        if(startSock) {
            edge->setStartSocket(startSock);
            startSock->addEdge(edge);
        }
        else {
            edge->deleteLater();
            continue;
        }

        Socket *endSock = edge->findSockets(scene, edge->endPosition().x(), edge->endPosition().y());
        if(endSock) {
            edge->setEndSocket(endSock);
            endSock->addEdge(edge);
        }
        else {
            edge->deleteLater();
            continue;
        }
        edge->updateScale(scene->background()->viewScale());
        scene->addEdge(edge);
        pastedItem.append(edge);
    }
    scene->pastedItems(pastedItem);
}

void Clipboard::duplicate(Scene *scene) {
    QList<QQuickItem*> selected = scene->selectedList();
    float viewScale = scene->background()->viewScale();
    QList<Frame*> sel_frames;
    QList<Node*> sel_nodes;
    QList<Edge*> sel_edges;
    QList<QQuickItem*> pastedItem;
    for(auto item: selected) {
        if(!(qobject_cast<AlbedoNode*>(item) || qobject_cast<MetalNode*>(item) || qobject_cast<RoughNode*>(item) || qobject_cast<NormalNode*>(item) || qobject_cast<HeightNode*>(item) || qobject_cast<EmissionNode*>(item))) {
            if(qobject_cast<Node*>(item)) {
                Node *node = qobject_cast<Node*>(item);
                if(!node->attachedFrame() || !node->attachedFrame()->selected()) sel_nodes.append(node);
                QList<Edge*> edges = node->getEdges();
                for(auto edge: edges) {
                    if(sel_edges.contains(edge)) continue;
                    Node *startNode = qobject_cast<Node*>(edge->startSocket()->parentItem());
                    Node *endNode = qobject_cast<Node*>(edge->endSocket()->parentItem());
                    if((startNode && startNode->selected()) && (!(qobject_cast<AlbedoNode*>(endNode) || qobject_cast<MetalNode*>(endNode) || qobject_cast<RoughNode*>(endNode) || qobject_cast<NormalNode*>(endNode) || qobject_cast<HeightNode*>(endNode) || qobject_cast<EmissionNode*>(endNode)) && endNode->selected())) {
                        sel_edges.append(edge);
                    }
                }
            }
            else if(qobject_cast<Frame*>(item)) {
                Frame *frame = qobject_cast<Frame*>(item);
                sel_frames.append(frame);
            }
        }
    }

    for(auto frame: sel_frames) {
        Frame *f = new Frame(scene);
        f->setBaseX(frame->baseX() + 50);
        f->setBaseY(frame->baseY() + 50);
        f->setWidth(frame->width());
        f->setHeight(frame->height());
        f->setTitle(frame->title());
        f->setColor(frame->color());
        scene->addFrame(f);
        scene->addSelected(f);
        pastedItem.append(f);
        QList<QQuickItem*> copiedContent;
        for(auto item: frame->contentList()) {
            Node *baseNode = qobject_cast<Node*>(item);
            if(baseNode && !(qobject_cast<AlbedoNode*>(item) || qobject_cast<MetalNode*>(item) || qobject_cast<RoughNode*>(item) || qobject_cast<NormalNode*>(item) || qobject_cast<HeightNode*>(item) || qobject_cast<EmissionNode*>(item)) && baseNode->selected()) {
                Node *copiedContentNode = baseNode->clone();
                copiedContentNode->setBaseX(baseNode->baseX() + 50);
                copiedContentNode->setBaseY(baseNode->baseY() + 50);
                copiedContent.append(copiedContentNode);
                scene->addNode(copiedContentNode);
                scene->addSelected(copiedContentNode);
                pastedItem.append(copiedContentNode);
            }
        }
        if(copiedContent.size() > 0) f->addNodes(copiedContent);
    }
    for(auto node: sel_nodes) {
        Node *duplicatedNode = node->clone();
        duplicatedNode->setBaseX(node->baseX() + 50);
        duplicatedNode->setBaseY(node->baseY() + 50);
        scene->addNode(duplicatedNode);
        scene->addSelected(duplicatedNode);
        pastedItem.append(duplicatedNode);
    }
    for(auto edge: sel_edges) {
        Edge *e = new Edge(scene);
        e->setStartPosition(edge->startPosition() + QVector2D(50, 50)*viewScale);
        e->setEndPosition(edge->endPosition() + QVector2D(50, 50)*viewScale);
        Socket *startSock = e->findSockets(scene, e->startPosition().x(), e->startPosition().y());
        if(startSock) {
            e->setStartSocket(startSock);
            startSock->addEdge(e);
        }

        Socket *endSock = e->findSockets(scene, e->endPosition().x(), e->endPosition().y());
        if(endSock) {
            e->setEndSocket(endSock);
            endSock->addEdge(e);
        }
        e->updateScale(scene->background()->viewScale());
        scene->addEdge(e);
        pastedItem.append(e);
    }
    scene->pastedItems(pastedItem);
}

void Clipboard::clear() {
    for(auto frame: clipboard_frames) {
        if(frame) {
            for(auto item: frame->contentList()) {
                delete item;
            }
            delete frame;
        }
    }
    clipboard_frames.clear();
    for(auto node: clipboard_nodes){
        if(node) delete node;
    }
    clipboard_nodes.clear();
    for(auto edge: clipboard_edges) {
        if(edge) delete edge;
    }
    clipboard_edges.clear();
}

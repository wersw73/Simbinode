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

#include "colorramp.h"
#include <QOpenGLFramebufferObjectFormat>
#include <iostream>

bool gradientSort(QVector4D f, QVector4D s) {
    return f.w() < s.w();
}

ColorRampObject::ColorRampObject(QQuickItem *parent, QVector2D resolution, QJsonArray stops):
    QQuickFramebufferObject (parent), m_resolution(resolution)
{
    setMirrorVertically(true);
    m_stops.clear();
    for(auto s: stops) {
        QJsonArray g = s.toArray();
        QVector4D grad = QVector4D(g[0].toVariant().toFloat(), g[1].toVariant().toFloat(), g[2].toVariant().toFloat(), g[3].toVariant().toFloat());
        m_stops.push_back(grad);
    }
}

QQuickFramebufferObject::Renderer *ColorRampObject::createRenderer() const {
    return new ColorRampRenderer(m_resolution);
}

unsigned int &ColorRampObject::texture() {
    return m_texture;
}

void ColorRampObject::setTexture(unsigned int texture) {
    m_texture = texture;
    textureChanged();
}

unsigned int ColorRampObject::maskTexture() {
    return m_maskTexture;
}

void ColorRampObject::setMaskTexture(unsigned int texture) {
    m_maskTexture = texture;
    rampedTex = true;
    update();
}

std::vector<QVector4D> &ColorRampObject::stops() {
    return m_stops;
}

unsigned int ColorRampObject::sourceTexture() {
    return  m_sourceTexture;
}

void ColorRampObject::setSourceTexture(unsigned int texture) {
    m_sourceTexture = texture;
    rampedTex = true;
    update();
}

void ColorRampObject::setGradientsStops(QJsonArray stops) {
    m_stops.clear();
    for(auto s: stops) {
        QJsonArray g = s.toArray();
        QVector4D grad = QVector4D(g[0].toVariant().toFloat(), g[1].toVariant().toFloat(), g[2].toVariant().toFloat(), g[3].toVariant().toFloat());
        m_stops.push_back(grad);
    }
    update();
}

QVector2D ColorRampObject::resolution() {
    return m_resolution;
}

void ColorRampObject::setResolution(QVector2D res) {
    m_resolution = res;
    resUpdated = true;
    update();
}

void ColorRampObject::gradientAdd(QVector3D color, qreal pos) {
    rampedTex = true;
    QVector4D grad = QVector4D(color, pos);
    m_stops.push_back(grad);
    update();
}

void ColorRampObject::positionUpdate(qreal pos, int index) {
    rampedTex = true;
    m_stops[index].setW(pos);
    update();
}

void ColorRampObject::colorUpdate(QVector3D color, int index) {
    rampedTex = true;
    m_stops[index].setX(color.x());
    m_stops[index].setY(color.y());
    m_stops[index].setZ(color.z());
    update();
}

void ColorRampObject::gradientDelete(int index) {
    rampedTex = true;
    m_stops.erase(m_stops.begin() + index);
    update();
}

ColorRampRenderer::ColorRampRenderer(QVector2D res): m_resolution(res) {
    initializeOpenGLFunctions();
    colorRampShader = new QOpenGLShaderProgram();
    colorRampShader->addCacheableShaderFromSourceFile(QOpenGLShader::Vertex, ":/shaders/texture.vert");
    colorRampShader->addCacheableShaderFromSourceFile(QOpenGLShader::Fragment, ":/shaders/colorramp.frag");
    colorRampShader->link();
    textureShader = new QOpenGLShaderProgram();
    textureShader->addCacheableShaderFromSourceFile(QOpenGLShader::Vertex, ":/shaders/texture.vert");
    textureShader->addCacheableShaderFromSourceFile(QOpenGLShader::Fragment, ":/shaders/texture.frag");
    textureShader->link();
    colorRampShader->bind();
    colorRampShader->setUniformValue(colorRampShader->uniformLocation("sourceTexture"), 0);
    colorRampShader->setUniformValue(colorRampShader->uniformLocation("maskTexture"), 1);
    colorRampShader->release();
    textureShader->bind();
    textureShader->setUniformValue(textureShader->uniformLocation("textureSample"), 0);
    textureShader->release();
    float vertQuadTex[] = {-1.0f, -1.0f, 0.0f, 0.0f,
                    -1.0f, 1.0f, 0.0f, 1.0f,
                    1.0f, -1.0f, 1.0f, 0.0f,
                    1.0f, 1.0f, 1.0f, 1.0f};
    unsigned int VBO;
    glGenVertexArrays(1, &textureVAO);
    glBindVertexArray(textureVAO);
    glGenBuffers(1, &VBO);
    glBindBuffer(GL_ARRAY_BUFFER, VBO);
    glBufferData(GL_ARRAY_BUFFER, sizeof(vertQuadTex), vertQuadTex, GL_STATIC_DRAW);
    glEnableVertexAttribArray(0);
    glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, 4*sizeof(float), nullptr);
    glEnableVertexAttribArray(1);
    glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 4*sizeof(float), (void*)(2*sizeof(float)));
    glBindBuffer(GL_ARRAY_BUFFER, 0);
    glBindVertexArray(0);
    glGenFramebuffers(1, &colorFBO);
    glGenTextures(1, &m_colorTexture);
    glBindFramebuffer(GL_FRAMEBUFFER, colorFBO);
    glBindTexture(GL_TEXTURE_2D, m_colorTexture);
    glTexImage2D(
        GL_TEXTURE_2D, 0, GL_RGBA, m_resolution.x(), m_resolution.y(), 0, GL_RGBA, GL_UNSIGNED_BYTE, nullptr
    );
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
    glFramebufferTexture2D(
        GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, m_colorTexture, 0
    );
    glBindTexture(GL_TEXTURE_2D, 0);
    glBindFramebuffer(GL_FRAMEBUFFER, 0);

    glGenBuffers(1, &gradientsSSBO);
}

ColorRampRenderer::~ColorRampRenderer() {
    delete colorRampShader;
    delete textureShader;
}

QOpenGLFramebufferObject *ColorRampRenderer::createFramebufferObject(const QSize &size) {
    QOpenGLFramebufferObjectFormat format;
    format.setAttachment(QOpenGLFramebufferObject::CombinedDepthStencil);
    format.setSamples(8);
    return new QOpenGLFramebufferObject(size, format);
}

void ColorRampRenderer::synchronize(QQuickFramebufferObject *item) {
    ColorRampObject *colorRampItem = static_cast<ColorRampObject*>(item);    
    if(colorRampItem->resUpdated) {
        colorRampItem->resUpdated = false;
        m_resolution = colorRampItem->resolution();
        updateTexResolution();
    }
    if(colorRampItem->rampedTex) {
        colorRampItem->rampedTex = false;

        m_sourceTexture = colorRampItem->sourceTexture();
        if(m_sourceTexture) {
            maskTexture = colorRampItem->maskTexture();
            colorRamp(colorRampItem->stops());
            if(colorRampItem->selectedItem) colorRampItem->updatePreview(m_colorTexture, true);
            colorRampItem->setTexture(m_colorTexture);
        }
    }    
}

void ColorRampRenderer::render() {
    glDisable(GL_DEPTH_TEST);
    glEnable(GL_BLEND);
    glBlendFuncSeparate(GL_ONE, GL_ZERO, GL_ZERO, GL_ONE);
    glClearColor(0.0f, 0.0f, 0.0f, 1.0f);
    glClear(GL_COLOR_BUFFER_BIT);
    if(m_sourceTexture) {
        glBindVertexArray(textureVAO);
        textureShader->bind();
        glActiveTexture(GL_TEXTURE0);
        glBindTexture(GL_TEXTURE_2D, m_colorTexture);
        glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);
        glBindTexture(GL_TEXTURE_2D, 0);
        textureShader->release();
        glBindVertexArray(0);
    }
}

void ColorRampRenderer::colorRamp(const std::vector<QVector4D> &stops) {
    glBindFramebuffer(GL_FRAMEBUFFER, colorFBO);
    glViewport(0, 0, m_resolution.x(), m_resolution.y());
    glClearColor(0.0f, 0.0f, 0.0f, 0.0f);
    glClear(GL_COLOR_BUFFER_BIT);
    std::vector<QVector4D> gradSort = stops;
    std::reverse(gradSort.begin(), gradSort.end());
    std::sort(gradSort.begin(), gradSort.end(), gradientSort);
    colorRampShader->bind();
    glBindVertexArray(textureVAO);
    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, m_sourceTexture);
    glActiveTexture(GL_TEXTURE1);
    glBindTexture(GL_TEXTURE_2D, maskTexture);
    glBindBuffer(GL_SHADER_STORAGE_BUFFER, gradientsSSBO);
    glBufferData(GL_SHADER_STORAGE_BUFFER, gradSort.size()*sizeof (QVector4D), gradSort.data(), GL_STATIC_DRAW);
    glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 0, gradientsSSBO);
    colorRampShader->setUniformValue(colorRampShader->uniformLocation("stopCount"), (int)gradSort.size());
    colorRampShader->setUniformValue(colorRampShader->uniformLocation("useMask"), maskTexture);
    glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);
    glBindBuffer(GL_SHADER_STORAGE_BUFFER, 0);
    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, 0);
    glBindVertexArray(0);
    colorRampShader->release();
    glBindFramebuffer(GL_FRAMEBUFFER, 0);
}

void ColorRampRenderer::updateTexResolution() {
    glBindTexture(GL_TEXTURE_2D, m_colorTexture);
    glTexImage2D(
        GL_TEXTURE_2D, 0, GL_RGBA, m_resolution.x(), m_resolution.y(), 0, GL_RGBA, GL_UNSIGNED_BYTE, nullptr
    );
    glBindTexture(GL_TEXTURE_2D, 0);
}


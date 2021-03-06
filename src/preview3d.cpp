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

#include "preview3d.h"
#include <QOpenGLFramebufferObjectFormat>
#include <iostream>
#include <QDir>

Preview3DObject::Preview3DObject(QQuickItem *parent): QQuickFramebufferObject (parent)
{
    setAcceptedMouseButtons(Qt::AllButtons);
    setAcceptHoverEvents(true);

    connect(&m_timer, &QTimer::timeout, this, &Preview3DObject::transformed);
}

QQuickFramebufferObject::Renderer *Preview3DObject::createRenderer() const {
    return new Preview3DRenderer();
}

void Preview3DObject::mousePressEvent(QMouseEvent *event) {
    if(event->button() == Qt::MidButton) {
        lastX = event->pos().x();
        lastY = event->pos().y();
        if(event->modifiers() == Qt::ShiftModifier) {
            QMatrix4x4 projection = QMatrix4x4();
            projection.perspective(m_zoomCam, (float)width()/(float)height(), 0.1f, 38.0f);
            QMatrix4x4 view = QMatrix4x4();
            view.translate(0, 0, 19);
            QMatrix4x4 model = QMatrix4x4();
            model.translate(0.0f, 0.0f, 0.0f);
            QVector3D z = QVector3D(0, 0, 0).project(model*view, projection, QRect(0, 0, width(), height()));
            QVector3D viewPos = QVector3D(lastX, lastY, z.z());
            lastWorldPos = viewPos.unproject(model*view, projection, QRect(0, 0, width(), height()));
        }
    }
    else if(event->button() == Qt::RightButton) {
        lastX = event->pos().x();
        lastY = event->pos().y();
    }
    else {
        event->ignore();
    }
}

void Preview3DObject::mouseMoveEvent(QMouseEvent *event) {
    if(event->buttons() == Qt::MidButton && event->modifiers() == Qt::ShiftModifier) {
        QMatrix4x4 projection = QMatrix4x4();
        projection.perspective(m_zoomCam, (float)width()/(float)height(), 0.1f, 38.0f);
        QMatrix4x4 view = QMatrix4x4();
        view.translate(0, 0, 19);
        QMatrix4x4 model = QMatrix4x4();
        model.translate(0.0f, 0.0f, 0.0f);
        QVector3D z = QVector3D(0, 0, 0).project(model*view, projection, QRect(0, 0, width(), height()));
        QVector3D viewPosNew = QVector3D(event->pos().x(), event->pos().y(), z.z());
        QVector3D worldPosNew = viewPosNew.unproject(model*view, projection, QRect(0, 0, width(), height()));
        QVector3D offset = QVector3D(lastWorldPos.x() - worldPosNew.x(), lastWorldPos.y() - worldPosNew.y(), 0);
        m_posCam += offset;
        lastX = event->pos().x();
        lastY = event->pos().y();
        lastWorldPos = worldPosNew;
        translationView = true;
        transformView = true;
        update();
    }
    else if(event->buttons() == Qt::MidButton) {
        theta += 0.01f*(lastX - event->pos().x());
        phi += 0.01f*(event->pos().y() - lastY);

        float x = 0;
        float y = sin(theta/2);
        float z = 0;
        float w = cos(theta/2);

        QQuaternion rotY = QQuaternion(w, x, y, z);

        x = sin(phi/2);
        y = 0;
        z = 0;
        w = cos(phi/2);

        QQuaternion rotX = QQuaternion(w, x, y, z);

        m_rotQuat = rotX * rotY;
        lastX = event->pos().x();
        lastY = event->pos().y();
        rotationObject = true;
        transformView = true;
        update();
    }
    else if(event->buttons() == Qt::RightButton && event->modifiers() == Qt::AltModifier) {
        float stepZoom = 0.0f;
        if(event->pos().x() - lastX > 0) {
           if((m_zoomCam - 0.05f*(event->pos().x() - lastX)) < 1.0f) stepZoom = m_zoomCam - 1.0f;
           else stepZoom = 0.05f*(event->pos().x() - lastX);
        }
        else {
            if((m_zoomCam - 0.05f*(event->pos().x() - lastX)) > 45.0f) stepZoom = m_zoomCam - 45.0f;
            else stepZoom = 0.05f*(event->pos().x() - lastX);
        }
        if(stepZoom != 0.0f) {
            m_zoomCam -= stepZoom;
            lastX = event->pos().x();
            lastY = event->pos().y();
            zoomView = true;
            transformView = true;
            update();
        }
    }
}

void Preview3DObject::mouseReleaseEvent(QMouseEvent *event) {
    if((event->button() == Qt::MidButton && event->modifiers() == Qt::ShiftModifier) ||
       event->button() == Qt::MidButton ||
       (event->button() == Qt::RightButton && event->modifiers() == Qt::AltModifier)) {
        transformView = false;
        update();
    }
}

void Preview3DObject::wheelEvent(QWheelEvent *event) {
    float stepZoom = 0.0f;
    if(event->angleDelta().y() > 0) {
       if((m_zoomCam - 0.01f*event->angleDelta().y()) < 1.0f) return;
       stepZoom = 0.01f*event->angleDelta().y();
    }
    else {
        if((m_zoomCam - 0.01f*event->angleDelta().y()) > 45.0f) return;
        stepZoom = 0.01f*event->angleDelta().y();
    }
    m_zoomCam -= stepZoom;
    zoomView = true;
    transformView = true;
    m_timer.start(500);
    update();
}

void Preview3DObject::keyPressEvent(QKeyEvent *event) {
    if(event->key() == Qt::Key_F) {
        m_posCam = QVector3D(0, 0, 19);
        translationView = true;
        update();
    }
}

void Preview3DObject::hoverEnterEvent(QHoverEvent *event) {
    setFocus(true);
}

void Preview3DObject::hoverLeaveEvent(QHoverEvent *event) {
    setFocus(false);
}

QVector3D Preview3DObject::posCam() {
    return m_posCam;
}

float Preview3DObject::zoomCam() {
    return m_zoomCam;
}

QQuaternion Preview3DObject::rotQuat() {
    return m_rotQuat;
}

int Preview3DObject::primitivesType() {
    return m_primitive;
}

void Preview3DObject::setPrimitivesType(int type) {
    m_primitive = type;
    theta = 0;
    phi = 0;
    m_rotQuat = QQuaternion();
    m_zoomCam = 35.0f;
    m_posCam = QVector3D(0, 0, 19);
    update();
}

int Preview3DObject::tilesSize() {
    return m_tile;
}

void Preview3DObject::setTilesSize(int id) {
    m_tile = id + 1;
    update();
}

float Preview3DObject::heightScale() {
    return m_heightScale;
}

void Preview3DObject::setHeightScale(float scale) {
    m_heightScale = scale;
    update();
}

float Preview3DObject::emissiveStrenght() {
    return m_emissiveStrenght;
}

void Preview3DObject::setEmissiveStrenght(float strenght) {
    m_emissiveStrenght = strenght;
    update();
}

float Preview3DObject::bloomRadius() {
    return m_bloomRadius;
}

void Preview3DObject::setBloomRadius(float radius) {
    m_bloomRadius = radius;
    update();
}

float Preview3DObject::bloomIntensity() {
    return m_bloomIntensity;
}

void Preview3DObject::setBloomIntensity(float intensity) {
    m_bloomIntensity = intensity;
    update();
}

float Preview3DObject::bloomThreshold() {
    return m_bloomThreshold;
}

void Preview3DObject::setBloomThreshold(float threshold) {
    m_bloomThreshold = threshold;
    update();
}

bool Preview3DObject::bloom() {
    return m_bloom;
}

void Preview3DObject::setBloom(bool enable) {
    m_bloom = enable;
    update();
}

void Preview3DObject::transformed() {
    m_timer.stop();
    if(!transformView) return;
    transformView = false;
    update();
}

QVariant Preview3DObject::albedo() {
    return m_albedo;
}

QVariant Preview3DObject::metalness() {
    return m_metalness;
}

QVariant Preview3DObject::roughness() {
    return m_roughness;
}

unsigned int Preview3DObject::normal() {
    return m_normal;
}

unsigned int Preview3DObject::heightMap() {
    return m_height;
}

unsigned int Preview3DObject::emission() {
    return m_emission;
}

QVector2D Preview3DObject::texResolution() {
    return m_texResolution;
}

void Preview3DObject::setTexResolution(QVector2D res) {
    m_texResolution = res;
    updateRes = true;
    update();
}

void Preview3DObject::updateAlbedo(QVariant albedo, bool useTexture) {
    useAlbedoTex = useTexture;
    m_albedo = albedo;
    changedAlbedo = true;
    update();
}

void Preview3DObject::updateMetal(QVariant metal, bool useTexture) {
    useMetalTex = useTexture;
    m_metalness = metal;
    changedMetal = true;
    update();
}

void Preview3DObject::updateRough(QVariant rough, bool useTexture) {
    useRoughTex = useTexture;
    m_roughness = rough;
    changedRough = true;
    update();
}

void Preview3DObject::updateNormal(unsigned int normal) {
    m_normal = normal;
    useNormTex = normal;
    changedNormal = true;
    update();
}

void Preview3DObject::updateHeight(unsigned int height) {
    m_height = height;
    changedHeight = true;
    update();
}

void Preview3DObject::updateEmission(unsigned int emission) {
    m_emission = emission;
    changedEmission = true;
    update();
}

void Preview3DObject::sizeUpdated(bool resize) {
    if(resized == resize) return;
    resized = resize;
    transformView = resized;
    update();
}

Preview3DRenderer::Preview3DRenderer() {
    initializeOpenGLFunctions();

    pbrShader = new QOpenGLShaderProgram();
    pbrShader->addCacheableShaderFromSourceFile(QOpenGLShader::Vertex, ":/shaders/pbr.vert");
    pbrShader->addCacheableShaderFromSourceFile(QOpenGLShader::Fragment, ":/shaders/pbr.frag");
    pbrShader->link();

    equirectangularShader = new QOpenGLShaderProgram();
    equirectangularShader->addCacheableShaderFromSourceFile(QOpenGLShader::Vertex, ":/shaders/cubemap.vert");
    equirectangularShader->addCacheableShaderFromSourceFile(QOpenGLShader::Fragment, ":/shaders/equirectangular.frag");
    equirectangularShader->link();

    irradianceShader = new QOpenGLShaderProgram();
    irradianceShader->addCacheableShaderFromSourceFile(QOpenGLShader::Vertex, ":/shaders/cubemap.vert");
    irradianceShader->addCacheableShaderFromSourceFile(QOpenGLShader::Fragment, ":/shaders/irradiance.frag");
    irradianceShader->link();

    prefilteredShader = new QOpenGLShaderProgram();
    prefilteredShader->addCacheableShaderFromSourceFile(QOpenGLShader::Vertex, ":/shaders/cubemap.vert");
    prefilteredShader->addCacheableShaderFromSourceFile(QOpenGLShader::Fragment, ":/shaders/prefiltered.frag");
    prefilteredShader->link();

    brdfShader = new QOpenGLShaderProgram();
    brdfShader->addCacheableShaderFromSourceFile(QOpenGLShader::Vertex, ":/shaders/brdf.vert");
    brdfShader->addCacheableShaderFromSourceFile(QOpenGLShader::Fragment, ":/shaders/brdf.frag");
    brdfShader->link();

    backgroundShader = new QOpenGLShaderProgram();
    backgroundShader->addCacheableShaderFromSourceFile(QOpenGLShader::Vertex, ":/shaders/background.vert");
    backgroundShader->addCacheableShaderFromSourceFile(QOpenGLShader::Fragment, ":/shaders/background.frag");
    backgroundShader->link();

    textureShader = new QOpenGLShaderProgram();
    textureShader->addCacheableShaderFromSourceFile(QOpenGLShader::Vertex, ":/shaders/brdf.vert");
    textureShader->addCacheableShaderFromSourceFile(QOpenGLShader::Fragment, ":/shaders/texture.frag");
    textureShader->link();

    blurShader = new QOpenGLShaderProgram();
    blurShader->addCacheableShaderFromSourceFile(QOpenGLShader::Vertex, ":/shaders/texture.vert");
    blurShader->addCacheableShaderFromSourceFile(QOpenGLShader::Fragment, ":/shaders/brightblur.frag");
    blurShader->link();

    bloomShader = new QOpenGLShaderProgram();
    bloomShader->addCacheableShaderFromSourceFile(QOpenGLShader::Vertex, ":/shaders/texture.vert");
    bloomShader->addCacheableShaderFromSourceFile(QOpenGLShader::Fragment, ":/shaders/applybloom.frag");
    bloomShader->link();

    brightShader = new QOpenGLShaderProgram();
    brightShader->addCacheableShaderFromSourceFile(QOpenGLShader::Vertex, ":/shaders/texture.vert");
    brightShader->addCacheableShaderFromSourceFile(QOpenGLShader::Fragment, ":/shaders/brightforbloom.frag");
    brightShader->link();

    pbrShader->bind();
    pbrShader->setUniformValue(pbrShader->uniformLocation("irradianceMap"), 0);
    pbrShader->setUniformValue(pbrShader->uniformLocation("prefilterMap"), 1);
    pbrShader->setUniformValue(pbrShader->uniformLocation("brdfLUT"), 2);
    pbrShader->setUniformValue(pbrShader->uniformLocation("albedoMap"), 3);
    pbrShader->setUniformValue(pbrShader->uniformLocation("normalMap"), 4);
    pbrShader->setUniformValue(pbrShader->uniformLocation("metallicMap"), 5);
    pbrShader->setUniformValue(pbrShader->uniformLocation("roughnessMap"), 6);
    pbrShader->setUniformValue(pbrShader->uniformLocation("heightMap"), 7);
    pbrShader->setUniformValue(pbrShader->uniformLocation("emissionMap"), 8);
    pbrShader->setUniformValue(pbrShader->uniformLocation("albedoVal"), QVector3D(1.0f, 1.0f, 1.0f));
    pbrShader->setUniformValue(pbrShader->uniformLocation("metallicVal"), 0.0f);
    pbrShader->setUniformValue(pbrShader->uniformLocation("roughnessVal"), 0.2f);
    pbrShader->setUniformValue(pbrShader->uniformLocation("cameraPos"), positionV);
    pbrShader->setUniformValue(pbrShader->uniformLocation("res"), m_texResolution.x());
    pbrShader->release();

    backgroundShader->bind();
    backgroundShader->setUniformValue(backgroundShader->uniformLocation("environmentMap"), 0);
    backgroundShader->release();

    textureShader->bind();
    textureShader->setUniformValue(textureShader->uniformLocation("textureSample"), 0);
    textureShader->release();

    blurShader->bind();
    blurShader->setUniformValue(blurShader->uniformLocation("image"), 0);
    blurShader->release();

    bloomShader->bind();
    bloomShader->setUniformValue(bloomShader->uniformLocation("scene"), 0);
    bloomShader->setUniformValue(bloomShader->uniformLocation("bloomBlur"), 1);
    bloomShader->release();

    brightShader->bind();
    brightShader->setUniformValue(brightShader->uniformLocation("textureSample"), 0);
    brightShader->release();

    glEnable(GL_DEPTH_TEST);
    glDepthFunc(GL_LEQUAL);
    glEnable(GL_TEXTURE_CUBE_MAP_SEAMLESS);

    unsigned int captureFBO;
    unsigned int captureRBO;
    glGenFramebuffers(1, &captureFBO);
    glGenRenderbuffers(1, &captureRBO);
    glBindFramebuffer(GL_FRAMEBUFFER, captureFBO);
    glBindRenderbuffer(GL_RENDERBUFFER, captureRBO);
    glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH_COMPONENT24, 512, 512);
    glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_RENDERBUFFER, captureRBO);
    glBindFramebuffer(GL_FRAMEBUFFER, 0);
    glBindRenderbuffer(GL_RENDERBUFFER, 0);

    FREE_IMAGE_FORMAT fif = FIF_UNKNOWN;
    FIBITMAP *dib(nullptr);
    BYTE* bits(nullptr);
    unsigned int width(0), height(0);
    QString hdrPath = QCoreApplication::applicationDirPath() + "/hdr/Newport_Loft_Ref.hdr";
    fif = FreeImage_GetFileType(hdrPath.toStdString().c_str(), 0);
    if (fif == FIF_UNKNOWN) {
        std::cout << "not get file type" << std::endl;
    }
    dib = FreeImage_Load(fif, hdrPath.toStdString().c_str());
    if (!dib) {
        std::cout << "not load image" << std::endl;
    }
    bits = FreeImage_GetBits(dib);
    width = FreeImage_GetWidth(dib);
    height = FreeImage_GetHeight(dib);
    glGenTextures(1, &hdrTexture);
    glBindTexture(GL_TEXTURE_2D, hdrTexture);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB16F, width, height, 0, GL_RGB, GL_FLOAT, (void*)bits);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glBindTexture(GL_TEXTURE_2D, 0);
    FreeImage_Unload(dib);

    QMatrix4x4 captureProjection;
    captureProjection.perspective(90.0f, 1.0f, 0.1f, 10.0f);
    QMatrix4x4 captureView[6];
    captureView[0].lookAt(QVector3D(0.0f, 0.0f, 0.0f), QVector3D(1.0f, 0.0f, 0.0f), QVector3D(0.0f, -1.0f, 0.0f));
    captureView[1].lookAt(QVector3D(0.0f, 0.0f, 0.0f), QVector3D(-1.0f, 0.0f, 0.0f), QVector3D(0.0f, -1.0f, 0.0f));
    captureView[2].lookAt(QVector3D(0.0f, 0.0f, 0.0f), QVector3D(0.0f, 1.0f, 0.0f), QVector3D(0.0f, 0.0f, 1.0f));
    captureView[3].lookAt(QVector3D(0.0f, 0.0f, 0.0f), QVector3D(0.0f, -1.0f, 0.0f), QVector3D(0.0f, 0.0f, -1.0f));
    captureView[4].lookAt(QVector3D(0.0f, 0.0f, 0.0f), QVector3D(0.0f, 0.0f, 1.0f), QVector3D(0.0f, -1.0f, 0.0f));
    captureView[5].lookAt(QVector3D(0.0f, 0.0f, 0.0f), QVector3D(0.0f, 0.0f, -1.0f), QVector3D(0.0f, -1.0f, 0.0f));

    std::cout << "env create" << std::endl;
    glGenTextures(1, &envCubemap);
    glBindTexture(GL_TEXTURE_CUBE_MAP, envCubemap);
    for (unsigned int i = 0; i < 6; ++i)
    {
        glTexImage2D(GL_TEXTURE_CUBE_MAP_POSITIVE_X + i, 0, GL_RGB16F, 512, 512, 0, GL_RGB, GL_FLOAT, nullptr);
    }
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_R, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    equirectangularShader->bind();
    equirectangularShader->setUniformValue(equirectangularShader->uniformLocation("equirectangularMap"), 0);
    equirectangularShader->setUniformValue(equirectangularShader->uniformLocation("projection"), captureProjection);
    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, hdrTexture);
    glViewport(0, 0, 512, 512);
    glBindFramebuffer(GL_FRAMEBUFFER, captureFBO);
    for(unsigned int i = 0; i < 6; ++i) {
        equirectangularShader->setUniformValue(equirectangularShader->uniformLocation("view"), captureView[i]);
        glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_CUBE_MAP_POSITIVE_X + i, envCubemap, 0);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
        renderCube();
    }
    equirectangularShader->release();
    glBindFramebuffer(GL_FRAMEBUFFER, 0);
    glBindTexture(GL_TEXTURE_CUBE_MAP, envCubemap);
    glGenerateMipmap(GL_TEXTURE_CUBE_MAP);
    glBindTexture(GL_TEXTURE_2D, 0);
    glBindTexture(GL_TEXTURE_CUBE_MAP, 0);

    //irradiance cubemap
    glBindFramebuffer(GL_FRAMEBUFFER, captureFBO);
    glBindRenderbuffer(GL_RENDERBUFFER, captureRBO);
    glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH_COMPONENT24, 32, 32);
    glGenTextures(1, &irradianceMap);
    glBindTexture(GL_TEXTURE_CUBE_MAP, irradianceMap);
    for (unsigned int i = 0; i < 6; ++i)
    {
        glTexImage2D(GL_TEXTURE_CUBE_MAP_POSITIVE_X + i, 0, GL_RGB16F, 32, 32, 0, GL_RGB, GL_FLOAT, nullptr);
    }
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_R, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    irradianceShader->bind();
    irradianceShader->setUniformValue(irradianceShader->uniformLocation("projection"), captureProjection);
    glActiveTexture(GL_TEXTURE0);
    irradianceShader->setUniformValue(irradianceShader->uniformLocation("environmentMap"), 0);
    glBindTexture(GL_TEXTURE_CUBE_MAP, envCubemap);
    glViewport(0, 0, 32, 32);
    glBindFramebuffer(GL_FRAMEBUFFER, captureFBO);
    for(unsigned int i = 0; i < 6; ++i) {
        irradianceShader->setUniformValue(irradianceShader->uniformLocation("view"), captureView[i]);
        glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_CUBE_MAP_POSITIVE_X + i, irradianceMap, 0);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
        renderCube();
    }
    irradianceShader->release();
    glBindFramebuffer(GL_FRAMEBUFFER, 0);

    //prefiltered cubemap
    glGenTextures(1, &prefilterMap);
    glBindTexture(GL_TEXTURE_CUBE_MAP, prefilterMap);
    for (unsigned int i = 0; i < 6; ++i)
    {
        glTexImage2D(GL_TEXTURE_CUBE_MAP_POSITIVE_X + i, 0, GL_RGB16F, 128, 128, 0, GL_RGB, GL_FLOAT, nullptr);
    }
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_R, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_BASE_LEVEL, 0);
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MAX_LEVEL, 4);
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MAX_LOD, 4);
    glGenerateMipmap(GL_TEXTURE_CUBE_MAP);
    unsigned int maxMipLevels = 5;
    for(unsigned int mip = 0; mip < maxMipLevels; ++mip) {
        unsigned int mipWidth = 128 * std::pow(0.5, mip);
        unsigned int mipHeight = 128 * std::pow(0.5, mip);
        prefilteredShader->bind();
        prefilteredShader->setUniformValue(prefilteredShader->uniformLocation("environmentMap"), 0);
        prefilteredShader->setUniformValue(prefilteredShader->uniformLocation("projection"), captureProjection);
        glActiveTexture(GL_TEXTURE0);
        glBindTexture(GL_TEXTURE_CUBE_MAP, envCubemap);

        glBindFramebuffer(GL_FRAMEBUFFER, captureFBO);

        glBindRenderbuffer(GL_RENDERBUFFER, captureRBO);
        glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH_COMPONENT24, mipWidth, mipHeight);
        glViewport(0, 0, mipWidth, mipHeight);

        float roughness = (float)mip / (float)(maxMipLevels - 1);
        prefilteredShader->setUniformValue(prefilteredShader->uniformLocation("roughness"), roughness);
        for(unsigned int i = 0; i < 6; ++i) {
            prefilteredShader->setUniformValue(prefilteredShader->uniformLocation("view"), captureView[i]);
            glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_CUBE_MAP_POSITIVE_X + i, prefilterMap, mip);
            glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
            renderCube();
        }
        prefilteredShader->release();
        glBindFramebuffer(GL_FRAMEBUFFER, 0);
    }
    glBindTexture(GL_TEXTURE_CUBE_MAP, 0);

    glDisable(GL_BLEND);
    glGenTextures(1, &brdfLUTTexture);
    glBindTexture(GL_TEXTURE_2D, brdfLUTTexture);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RG16F, 512, 512, 0, GL_RG, GL_FLOAT, nullptr);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glBindFramebuffer(GL_FRAMEBUFFER, captureFBO);
    glBindRenderbuffer(GL_RENDERBUFFER, captureRBO);
    glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH_COMPONENT24, 512, 512);
    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, brdfLUTTexture, 0);
    glViewport(0, 0, 512, 512);
    brdfShader->bind();
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    renderQuad();
    brdfShader->release();
    glBindTexture(GL_TEXTURE_2D, 0);
    glBindFramebuffer(GL_FRAMEBUFFER, 0);
    glDeleteFramebuffers(1, &captureFBO);
    glDeleteRenderbuffers(1, &captureRBO);

    //buffer for rendering the scene and highlights
    glGenFramebuffers(1, &hdrFBO);
    glBindFramebuffer(GL_FRAMEBUFFER, hdrFBO);
    glGenTextures(1, &brightTexture);
    glBindTexture(GL_TEXTURE_2D, brightTexture);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, brightTexture, 0);
    glBindFramebuffer(GL_FRAMEBUFFER, 0);

    //FBO with multisample
    glGenFramebuffers(1, &multisampleFBO);
    glBindFramebuffer(GL_FRAMEBUFFER, multisampleFBO);
    glGenTextures(1, &multisampleTexture);
    glBindTexture(GL_TEXTURE_2D_MULTISAMPLE, multisampleTexture);
    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D_MULTISAMPLE, multisampleTexture, 0);
    glBindTexture(GL_TEXTURE_2D_MULTISAMPLE, 0);
    glGenRenderbuffers(1, &rboDepth);
    glBindRenderbuffer(GL_RENDERBUFFER, rboDepth);
    glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_RENDERBUFFER, rboDepth);
    glBindFramebuffer(GL_FRAMEBUFFER, 0);

    //FBO for copying multisample
    glGenFramebuffers(1, &screenFBO);
    glBindFramebuffer(GL_FRAMEBUFFER, screenFBO);
    glGenTextures(1, &screenTexture);
    glBindTexture(GL_TEXTURE_2D, screenTexture);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, screenTexture, 0);
    glBindTexture(GL_TEXTURE_2D, 0);
    glBindFramebuffer(GL_FRAMEBUFFER, 0);

    //FBO for blur
    glGenFramebuffers(2, pingpongFBO);
    glGenTextures(2, pingpongBuffer);
    for (unsigned int i = 0; i < 2; i++)
    {
        glBindFramebuffer(GL_FRAMEBUFFER, pingpongFBO[i]);
        glBindTexture(GL_TEXTURE_2D, pingpongBuffer[i]);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
        glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, pingpongBuffer[i], 0);
    }
    glBindTexture(GL_TEXTURE_2D, 0);
    glBindFramebuffer(GL_FRAMEBUFFER, 0);

    glGenTextures(1, &albedoTexture);
    glBindTexture(GL_TEXTURE_2D, albedoTexture);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST_MIPMAP_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, 1024, 1024, 0, GL_RGBA, GL_UNSIGNED_BYTE, nullptr);
    glBindTexture(GL_TEXTURE_2D, 0);

    glGenTextures(1, &normalTexture);
    glBindTexture(GL_TEXTURE_2D, normalTexture);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST_MIPMAP_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, 1024, 1024, 0, GL_RGBA, GL_UNSIGNED_BYTE, nullptr);
    glBindTexture(GL_TEXTURE_2D, 0);

    glGenTextures(1, &metalTexture);
    glBindTexture(GL_TEXTURE_2D, metalTexture);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST_MIPMAP_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, 1024, 1024, 0, GL_RGBA, GL_UNSIGNED_BYTE, nullptr);
    glBindTexture(GL_TEXTURE_2D, 0);

    glGenTextures(1, &roughTexture);
    glBindTexture(GL_TEXTURE_2D, roughTexture);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST_MIPMAP_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, 1024, 1024, 0, GL_RGBA, GL_UNSIGNED_BYTE, nullptr);
    glBindTexture(GL_TEXTURE_2D, 0);

    glGenTextures(1, &heightTexture);
    glBindTexture(GL_TEXTURE_2D, heightTexture);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST_MIPMAP_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, 1024, 1024, 0, GL_RGBA, GL_UNSIGNED_BYTE, nullptr);
    glBindTexture(GL_TEXTURE_2D, 0);

    glGenTextures(1, &emissionTexture);
    glBindTexture(GL_TEXTURE_2D, emissionTexture);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST_MIPMAP_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, 1024, 1024, 0, GL_RGBA, GL_UNSIGNED_BYTE, nullptr);
    glBindTexture(GL_TEXTURE_2D, 0);

    glGenFramebuffers(1, &outputFBO);
}

Preview3DRenderer::~Preview3DRenderer() {
    delete pbrShader;
    delete irradianceShader;
    delete backgroundShader;
    delete equirectangularShader;
    delete prefilteredShader;
    delete textureShader;
    delete brdfShader;
    delete blurShader;
    delete bloomShader;
    delete brightShader;
    glDeleteTextures(1, &hdrTexture);
    glDeleteTextures(1, &envCubemap);
    glDeleteTextures(1, &irradianceMap);
    glDeleteTextures(1, &prefilterMap);
    glDeleteTextures(1, &brdfLUTTexture);
    glDeleteTextures(1, &brightTexture);
    glDeleteTextures(2, &pingpongBuffer[0]);
    glDeleteTextures(1, &screenTexture);
    glDeleteTextures(1, &multisampleTexture);
    glDeleteTextures(1, &combinedTexture);
    glDeleteTextures(1, &albedoTexture);
    glDeleteTextures(1, &metalTexture);
    glDeleteTextures(1, &roughTexture);
    glDeleteTextures(1, &normalTexture);
    glDeleteTextures(1, &heightTexture);
    glDeleteTextures(1, &emissionTexture);
    glDeleteVertexArrays(1, &cubeMapVAO);
    glDeleteVertexArrays(1, &sphereVAO);
    glDeleteVertexArrays(1, &cubeVAO);
    glDeleteVertexArrays(1, &quadVAO);
    glDeleteVertexArrays(1, &planeVAO);
    glDeleteFramebuffers(1, &hdrFBO);
    glDeleteFramebuffers(2, &pingpongFBO[0]);
    glDeleteFramebuffers(1, &screenFBO);
    glDeleteFramebuffers(1, &multisampleFBO);
    glDeleteFramebuffers(1, &combinedFBO);
    glDeleteFramebuffers(1, &outputFBO);
    glDeleteRenderbuffers(1, &rboDepth);

}

QOpenGLFramebufferObject *Preview3DRenderer::createFramebufferObject(const QSize &size) {
    QOpenGLFramebufferObjectFormat format;
    format.setAttachment(QOpenGLFramebufferObject::CombinedDepthStencil);
    format.setSamples(samples);
    updateMatrix();
    return new QOpenGLFramebufferObject(size, format);
}

void Preview3DRenderer::synchronize(QQuickFramebufferObject *item) {
    Preview3DObject *previewItem = static_cast<Preview3DObject*>(item);

    if(wWidth != previewItem->width() || wHeight != previewItem->height()) {
        wWidth = previewItem->width();
        wHeight = previewItem->height();
        glBindTexture(GL_TEXTURE_2D, brightTexture);
        glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA16F, wWidth, wHeight, 0, GL_RGBA, GL_FLOAT, nullptr);
        glBindTexture(GL_TEXTURE_2D, screenTexture);
        glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA16F, wWidth, wHeight, 0, GL_RGBA, GL_FLOAT, nullptr);
        glBindTexture(GL_TEXTURE_2D, 0);
        glBindTexture(GL_TEXTURE_2D_MULTISAMPLE, multisampleTexture);
        glTexImage2DMultisample(GL_TEXTURE_2D_MULTISAMPLE, 8, GL_RGBA16F, wWidth, wHeight, GL_TRUE);
        glBindTexture(GL_TEXTURE_2D_MULTISAMPLE, 0);
        glBindRenderbuffer(GL_RENDERBUFFER, rboDepth);
        glRenderbufferStorageMultisample(GL_RENDERBUFFER, 8, GL_DEPTH_COMPONENT24, wWidth, wHeight);
        glBindRenderbuffer(GL_RENDERBUFFER, 0);

        updateMatrix();
    }

    if(primitive != previewItem->primitivesType()) {
        primitive = previewItem->primitivesType();
        positionV = previewItem->posCam();
        zoom = previewItem->zoomCam();
        rotQuat = previewItem->rotQuat();
        updateMatrix();
        pbrShader->setUniformValue(pbrShader->uniformLocation("cameraPos"), positionV);
    }
    if(previewItem->translationView) {
        previewItem->translationView = false;
        positionV = previewItem->posCam();
        updateMatrix();
        pbrShader->setUniformValue(pbrShader->uniformLocation("cameraPos"), positionV);
    }
    if(previewItem->zoomView) {
        previewItem->zoomView = false;
        zoom = previewItem->zoomCam();
        updateMatrix();
    }
    if(previewItem->rotationObject) {
        previewItem->rotationObject = false;
        rotQuat = previewItem->rotQuat();
        updateMatrix();
    }
    pbrShader->bind();
    if(previewItem->updateRes) {
        previewItem->updateRes = false;
        m_texResolution = previewItem->texResolution();
        pbrShader->setUniformValue(pbrShader->uniformLocation("resolution"), m_texResolution);
    }

    //reduce the number of samples when transforming the view
    if(previewItem->transformView != transformating) {
        transformating = previewItem->transformView;
        pbrShader->setUniformValue(pbrShader->uniformLocation("transformating"), transformating);
        samples = transformating ? 4 : 16;
        if(!previewItem->resized) invalidateFramebufferObject();
    }

    if(tilesSize != previewItem->tilesSize()) {
        tilesSize = previewItem->tilesSize();
        pbrShader->setUniformValue(pbrShader->uniformLocation("tilesSize"), tilesSize);
    }
    if(heightScale != previewItem->heightScale()) {
        heightScale = previewItem->heightScale();
        pbrShader->setUniformValue(pbrShader->uniformLocation("heightScale"), heightScale);
    }
    if(emissiveStrength != previewItem->emissiveStrenght()) {
        emissiveStrength = previewItem->emissiveStrenght();
        pbrShader->setUniformValue(pbrShader->uniformLocation("emissiveStrenght"), emissiveStrength);
    }
    if(bloom != previewItem->bloom()) {
        bloom = previewItem->bloom();
        pbrShader->setUniformValue(pbrShader->uniformLocation("bloom"), bloom);
    }

    if(useAlbedoTex != previewItem->useAlbedoTex) {
        useAlbedoTex = previewItem->useAlbedoTex;
        pbrShader->setUniformValue(pbrShader->uniformLocation("useAlbMap"), useAlbedoTex);
    }
    if(previewItem->changedAlbedo) {
        previewItem->changedAlbedo = false;
        if(previewItem->useAlbedoTex) {
            updateOutputsTexture(albedoTexture, previewItem->albedo().toUInt());
            pbrShader->bind();
        }
        else {
            pbrShader->setUniformValue(pbrShader->uniformLocation("albedoVal"), qvariant_cast<QVector3D>(previewItem->albedo()));
        }
    }

    if(useMetalTex != previewItem->useMetalTex) {
        useMetalTex = previewItem->useMetalTex;
        pbrShader->setUniformValue(pbrShader->uniformLocation("useMetalMap"), useMetalTex);
    }
    if(previewItem->changedMetal) {
        previewItem->changedMetal = false;
        if(previewItem->useMetalTex) {
            updateOutputsTexture(metalTexture, previewItem->metalness().toUInt());
            pbrShader->bind();
        }
        else {
            pbrShader->setUniformValue(pbrShader->uniformLocation("metallicVal"), previewItem->metalness().toFloat());
        }
    }

    if(useRoughTex != previewItem->useRoughTex) {
        useRoughTex = previewItem->useRoughTex;
        pbrShader->setUniformValue(pbrShader->uniformLocation("useRoughMap"), useRoughTex);
    }
    if(previewItem->changedRough) {
        previewItem->changedRough = false;
        if(previewItem->useRoughTex) {
            updateOutputsTexture(roughTexture, previewItem->roughness().toUInt());
            pbrShader->bind();
        }
        else {
            pbrShader->setUniformValue(pbrShader->uniformLocation("roughnessVal"), previewItem->roughness().toFloat());
        }
    }

    if(previewItem->changedNormal) {
        previewItem->changedNormal = false;
        updateOutputsTexture(normalTexture, previewItem->normal());
        pbrShader->bind();
    }
    bool useNorm = previewItem->normal() ? true : false;
    if(useNormalTex != useNorm) {
        useNormalTex = useNorm;
        pbrShader->setUniformValue(pbrShader->uniformLocation("useNormMap"), useNormalTex);
    }

    if(previewItem->changedHeight) {
        previewItem->changedHeight = false;
        updateOutputsTexture(heightTexture, previewItem->heightMap());
        pbrShader->bind();
    }
    bool useHeight = previewItem->heightMap();
    if(useHeightTex != useHeight) {
        useHeightTex = useHeight;
        pbrShader->setUniformValue(pbrShader->uniformLocation("useHeightMap"), useHeightTex);
    }

    if(previewItem->changedEmission) {
        previewItem->changedEmission = false;
        updateOutputsTexture(emissionTexture, previewItem->emission());
        pbrShader->bind();
    }
    if(useEmisTex != previewItem->emission()) {
        useEmisTex = previewItem->emission();
        pbrShader->setUniformValue(pbrShader->uniformLocation("useEmisMap"), useEmisTex);
    }

    pbrShader->release();

    bloom = previewItem->bloom();

    if(bloom) {
        bloomShader->bind();
        bloomShader->setUniformValue(bloomShader->uniformLocation("intensity"), previewItem->bloomIntensity());
        bloomShader->release();

        brightShader->bind();
        brightShader->setUniformValue(brightShader->uniformLocation("threshold"), previewItem->bloomThreshold());
        brightShader->release();

        bloomRadius = previewItem->bloomRadius();
        renderForBloom();
        brightnessBlur();
    }
}

void Preview3DRenderer::render() {
    if(bloom) {
        bloomShader->bind();
        glActiveTexture(GL_TEXTURE0);
        glBindTexture(GL_TEXTURE_2D, screenTexture);
        glActiveTexture(GL_TEXTURE1);
        glBindTexture(GL_TEXTURE_2D, pingpongBuffer[1]);
        renderQuad();
        glActiveTexture(GL_TEXTURE0);
        glBindTexture(GL_TEXTURE_2D, 0);
        bloomShader->release();
    }
    else {
        glEnable(GL_DEPTH_TEST);
        glDepthFunc(GL_LEQUAL);
        glEnable(GL_TEXTURE_CUBE_MAP_SEAMLESS);
        glClearColor(0.227f, 0.235f, 0.243f, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT | GL_STENCIL_BUFFER_BIT);

        renderScene();

        glDisable(GL_DEPTH_TEST);
    }
    glFlush();
    glFinish();

    /*glDepthFunc(GL_LEQUAL);
    backgroundShader->bind();
    backgroundShader->setUniformValue(backgroundShader->uniformLocation("projection"), projection);
    backgroundShader->setUniformValue(backgroundShader->uniformLocation("view"), view);
    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_CUBE_MAP, envCubemap);
    renderCube();
    backgroundShader->release();
    glBindTexture(GL_TEXTURE_CUBE_MAP, 0);*/
}

void Preview3DRenderer::renderCube() {
    if (!cubeVAO)
    {
        unsigned int cubeVBO;

        std::vector<QVector3D> positions;
        positions.push_back(QVector3D(-1.0f, -1.0f, -1.0f));
        positions.push_back(QVector3D(1.0f, 1.0f, -1.0f));
        positions.push_back(QVector3D(1.0f, -1.0f, -1.0f));
        positions.push_back(QVector3D(1.0f, 1.0f, -1.0f));
        positions.push_back(QVector3D(-1.0f, -1.0f, -1.0f));
        positions.push_back(QVector3D(-1.0f, 1.0f, -1.0f));

        positions.push_back(QVector3D(-1.0f, -1.0f, 1.0f));
        positions.push_back(QVector3D(1.0f, -1.0f, 1.0f));
        positions.push_back(QVector3D(1.0f, 1.0f, 1.0f));
        positions.push_back(QVector3D(1.0f, 1.0f, 1.0f));
        positions.push_back(QVector3D(-1.0f, 1.0f, 1.0f));
        positions.push_back(QVector3D(-1.0f, -1.0f, 1.0f));

        positions.push_back(QVector3D(-1.0f, 1.0f, 1.0f));
        positions.push_back(QVector3D(-1.0f, 1.0f, -1.0f));
        positions.push_back(QVector3D(-1.0f, -1.0f, -1.0f));
        positions.push_back(QVector3D(-1.0f, -1.0f, -1.0f));
        positions.push_back(QVector3D(-1.0f, -1.0f, 1.0f));
        positions.push_back(QVector3D(-1.0f, 1.0f, 1.0f));

        positions.push_back(QVector3D(1.0f, 1.0f, 1.0f));
        positions.push_back(QVector3D(1.0f, -1.0f, -1.0f));
        positions.push_back(QVector3D(1.0f, 1.0f, -1.0f));
        positions.push_back(QVector3D(1.0f, -1.0f, -1.0f));
        positions.push_back(QVector3D(1.0f, 1.0f, 1.0f));
        positions.push_back(QVector3D(1.0f, -1.0f, 1.0f));

        positions.push_back(QVector3D(-1.0f, -1.0f, -1.0f));
        positions.push_back(QVector3D(1.0f, -1.0f, -1.0f));
        positions.push_back(QVector3D(1.0f, -1.0f, 1.0f));
        positions.push_back(QVector3D(1.0f, -1.0f, 1.0f));
        positions.push_back(QVector3D(-1.0f, -1.0f, 1.0f));
        positions.push_back(QVector3D(-1.0f, -1.0f, -1.0f));

        positions.push_back(QVector3D(-1.0f, 1.0f, -1.0f));
        positions.push_back(QVector3D(1.0f, 1.0f, 1.0f));
        positions.push_back(QVector3D(1.0f, 1.0f, -1.0f));
        positions.push_back(QVector3D(1.0f, 1.0f, 1.0f));
        positions.push_back(QVector3D(-1.0f, 1.0f, -1.0f));
        positions.push_back(QVector3D(-1.0f, 1.0f, 1.0f));

        std::vector<QVector3D> normals;
        normals.push_back(QVector3D(0.0f, 0.0f, -1.0f));
        normals.push_back(QVector3D(0.0f, 0.0f, -1.0f));
        normals.push_back(QVector3D(0.0f, 0.0f, -1.0f));
        normals.push_back(QVector3D(0.0f, 0.0f, -1.0f));
        normals.push_back(QVector3D(0.0f, 0.0f, -1.0f));
        normals.push_back(QVector3D(0.0f, 0.0f, -1.0f));

        normals.push_back(QVector3D(0.0f, 0.0f, 1.0f));
        normals.push_back(QVector3D(0.0f, 0.0f, 1.0f));
        normals.push_back(QVector3D(0.0f, 0.0f, 1.0f));
        normals.push_back(QVector3D(0.0f, 0.0f, 1.0f));
        normals.push_back(QVector3D(0.0f, 0.0f, 1.0f));
        normals.push_back(QVector3D(0.0f, 0.0f, 1.0f));

        normals.push_back(QVector3D(-1.0f,  0.0f,  0.0f));
        normals.push_back(QVector3D(-1.0f,  0.0f,  0.0f));
        normals.push_back(QVector3D(-1.0f,  0.0f,  0.0f));
        normals.push_back(QVector3D(-1.0f,  0.0f,  0.0f));
        normals.push_back(QVector3D(-1.0f,  0.0f,  0.0f));
        normals.push_back(QVector3D(-1.0f,  0.0f,  0.0f));

        normals.push_back(QVector3D(1.0f,  0.0f,  0.0f));
        normals.push_back(QVector3D(1.0f,  0.0f,  0.0f));
        normals.push_back(QVector3D(1.0f,  0.0f,  0.0f));
        normals.push_back(QVector3D(1.0f,  0.0f,  0.0f));
        normals.push_back(QVector3D(1.0f,  0.0f,  0.0f));
        normals.push_back(QVector3D(1.0f,  0.0f,  0.0f));

        normals.push_back(QVector3D(0.0f,  -1.0f,  0.0f));
        normals.push_back(QVector3D(0.0f,  -1.0f,  0.0f));
        normals.push_back(QVector3D(0.0f,  -1.0f,  0.0f));
        normals.push_back(QVector3D(0.0f,  -1.0f,  0.0f));
        normals.push_back(QVector3D(0.0f,  -1.0f,  0.0f));
        normals.push_back(QVector3D(0.0f,  -1.0f,  0.0f));

        normals.push_back(QVector3D(0.0f,  1.0f,  0.0f));
        normals.push_back(QVector3D(0.0f,  1.0f,  0.0f));
        normals.push_back(QVector3D(0.0f,  1.0f,  0.0f));
        normals.push_back(QVector3D(0.0f,  1.0f,  0.0f));
        normals.push_back(QVector3D(0.0f,  1.0f,  0.0f));
        normals.push_back(QVector3D(0.0f,  1.0f,  0.0f));

        std::vector<QVector2D> uv;
        uv.push_back(QVector2D(0.0f, 1.0f));
        uv.push_back(QVector2D(1.0f, 0.0f));
        uv.push_back(QVector2D(1.0f, 1.0f));
        uv.push_back(QVector2D(1.0f, 0.0f));
        uv.push_back(QVector2D(0.0f, 1.0f));
        uv.push_back(QVector2D(0.0f, 0.0f));

        uv.push_back(QVector2D(1.0f, 1.0f));
        uv.push_back(QVector2D(0.0f, 1.0f));
        uv.push_back(QVector2D(0.0f, 0.0f));
        uv.push_back(QVector2D(0.0f, 0.0f));
        uv.push_back(QVector2D(1.0f, 0.0f));
        uv.push_back(QVector2D(1.0f, 1.0f));

        uv.push_back(QVector2D(0.0f, 0.0f));
        uv.push_back(QVector2D(1.0f, 0.0f));
        uv.push_back(QVector2D(1.0f, 1.0f));
        uv.push_back(QVector2D(1.0f, 1.0f));
        uv.push_back(QVector2D(0.0f, 1.0f));
        uv.push_back(QVector2D(0.0f, 0.0f));

        uv.push_back(QVector2D(1.0f, 0.0f));
        uv.push_back(QVector2D(0.0f, 1.0f));
        uv.push_back(QVector2D(0.0f, 0.0f));
        uv.push_back(QVector2D(0.0f, 1.0f));
        uv.push_back(QVector2D(1.0f, 0.0f));
        uv.push_back(QVector2D(1.0f, 1.0f));

        uv.push_back(QVector2D(0.0f, 0.0f));
        uv.push_back(QVector2D(1.0f, 0.0f));
        uv.push_back(QVector2D(1.0f, 1.0f));
        uv.push_back(QVector2D(1.0f, 1.0f));
        uv.push_back(QVector2D(0.0f, 1.0f));
        uv.push_back(QVector2D(0.0f, 0.0f));

        uv.push_back(QVector2D(1.0f, 0.0f));
        uv.push_back(QVector2D(0.0f, 1.0f));
        uv.push_back(QVector2D(0.0f, 0.0f));
        uv.push_back(QVector2D(0.0f, 1.0f));
        uv.push_back(QVector2D(1.0f, 0.0f));
        uv.push_back(QVector2D(1.0f, 1.0f));

        std::vector<QVector3D> tangent;
        for(unsigned int i = 0; i < positions.size(); i+= 3) {
            QVector3D pos0 = positions[i];
            QVector3D pos1 = positions[i + 1];
            QVector3D pos2 = positions[i + 2];
            QVector2D uv0 = uv[i];
            QVector2D uv1 = uv[i + 1];
            QVector2D uv2 = uv[i + 2];

            QVector3D e1 = pos1 - pos0;
            QVector3D e2 = pos2 - pos0;
            float x1 = uv1.x() - uv0.x();
            float x2 = uv2.x() - uv0.x();
            float y1 = uv1.y() - uv0.y();
            float y2 = uv2.y() - uv0.y();

            float f = 1.0f/(x1*y2 - x2*y1);
            QVector3D t = (e1*y2 - e2*y1)*f;
            tangent.push_back(t);
            tangent.push_back(t);
            tangent.push_back(t);
        }

        std::vector<float> data;
        for(unsigned int i = 0; i < positions.size(); ++i) {
            data.push_back(positions[i].x());
            data.push_back(positions[i].y());
            data.push_back(positions[i].z());
            if(normals.size() > 0) {
                data.push_back(normals[i].x());
                data.push_back(normals[i].y());
                data.push_back(normals[i].z());
            }
            if(uv.size() > 0) {
                data.push_back(uv[i].x());
                data.push_back(uv[i].y());
            }
            if(tangent.size() > 0) {
                data.push_back(tangent[i].x());
                data.push_back(tangent[i].y());
                data.push_back(tangent[i].z());
            }
        }

        glGenVertexArrays(1, &cubeVAO);
        glGenBuffers(1, &cubeVBO);

        glBindBuffer(GL_ARRAY_BUFFER, cubeVBO);
        glBufferData(GL_ARRAY_BUFFER, data.size() * sizeof(float), &data[0], GL_STATIC_DRAW);

        glBindVertexArray(cubeVAO);
        glEnableVertexAttribArray(0);
        glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 11 * sizeof(float), (void*)0);
        glEnableVertexAttribArray(1);
        glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 11 * sizeof(float), (void*)(3 * sizeof(float)));
        glEnableVertexAttribArray(2);
        glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, 11 * sizeof(float), (void*)(6 * sizeof(float)));
        glEnableVertexAttribArray(3);
        glVertexAttribPointer(3, 3, GL_FLOAT, GL_FALSE, 11 * sizeof(float), (void*)(8 * sizeof(float)));
        glBindBuffer(GL_ARRAY_BUFFER, 0);
    }

    glBindVertexArray(cubeVAO);
    glDrawArrays(GL_TRIANGLES, 0, 36);
    glBindVertexArray(0);
}

void Preview3DRenderer::renderQuad() {
    if (!quadVAO)
    {
        unsigned int quadVBO;

        float quadVertices[] = {
            -1.0f,  1.0f, 0.0f, 0.0f, 1.0f,
            -1.0f, -1.0f, 0.0f, 0.0f, 0.0f,
             1.0f,  1.0f, 0.0f, 1.0f, 1.0f,
             1.0f, -1.0f, 0.0f, 1.0f, 0.0f,
        };

        glGenVertexArrays(1, &quadVAO);
        glGenBuffers(1, &quadVBO);
        glBindVertexArray(quadVAO);
        glBindBuffer(GL_ARRAY_BUFFER, quadVBO);
        glBufferData(GL_ARRAY_BUFFER, sizeof(quadVertices), &quadVertices, GL_STATIC_DRAW);
        glEnableVertexAttribArray(0);
        glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 5 * sizeof(float), (void*)0);
        glEnableVertexAttribArray(1);
        glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 5 * sizeof(float), (void*)(3 * sizeof(float)));
        glBindBuffer(GL_ARRAY_BUFFER, 0);
    }
    glBindVertexArray(quadVAO);
    glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);
    glBindVertexArray(0);
}

void Preview3DRenderer::renderSphere() {
    if (!sphereVAO) {
        glGenVertexArrays(1, &sphereVAO);

        unsigned int vbo, ebo;
        glGenBuffers(1, &vbo);
        glGenBuffers(1, &ebo);

        std::vector<QVector3D> positions;
        std::vector<QVector2D> uv;
        std::vector<QVector3D> normals;
        std::vector<QVector3D> tangent;
        std::vector<QVector3D> bitangent;
        std::vector<unsigned int> indices;

        const unsigned int X_SEGMENTS = 64;
        const unsigned int Y_SEGMENTS = 64;
        const float PI = 3.14159265359f;
        for (unsigned int y = 0; y <= Y_SEGMENTS; ++y)
        {
            for (unsigned int x = 0; x <= X_SEGMENTS; ++x)
            {
                float xSegment = (float)x / (float)X_SEGMENTS;
                float ySegment = (float)y / (float)Y_SEGMENTS;
                float xPos = std::cos(xSegment * 2.0f * PI) * std::sin(ySegment * PI);
                float yPos = std::cos(ySegment * PI);
                float zPos = std::sin(xSegment * 2.0f * PI) * std::sin(ySegment * PI);

                positions.push_back(QVector3D(xPos, yPos, zPos));
                uv.push_back(QVector2D((2.0*xSegment), ySegment));
                normals.push_back(QVector3D(xPos, yPos, zPos));
            }
        }

       for (unsigned int y = 0; y < Y_SEGMENTS; ++y)
        {
            unsigned int start = y*(X_SEGMENTS + 1);
            unsigned int next = (y + 1)*(X_SEGMENTS + 1);
            for (unsigned int x = 0; x < X_SEGMENTS; ++x) {
                indices.push_back(start + x);
                indices.push_back(start + x + 1);
                indices.push_back(next + x);
                indices.push_back(next + x);
                indices.push_back(start + x + 1);                
                indices.push_back(next + x + 1);
            }
        }

        indexCount = indices.size();

        for(unsigned int i = 0; i < positions.size(); ++i) {
            tangent.push_back(QVector3D(0.0f, 0.0f, 0.0f));
            bitangent.push_back(QVector3D(0.0f, 0.0f, 0.0f));
        }

        for(unsigned int i = 0; i < indexCount; i += 3) {
            unsigned int idx0 = indices[i];
            unsigned int idx1 = indices[i + 1];
            unsigned int idx2 = indices[i + 2];
            QVector3D pos0 = positions[idx0];
            QVector3D pos1 = positions[idx1];
            QVector3D pos2 = positions[idx2];
            QVector3D uv0 = uv[idx0];
            QVector3D uv1 = uv[idx1];
            QVector3D uv2 = uv[idx2];

            QVector3D e1 = pos1 - pos0;
            QVector3D e2 = pos2 - pos0;
            float x1 = uv1.x() - uv0.x();
            float x2 = uv2.x() - uv0.x();
            float y1 = uv1.y() - uv0.y();
            float y2 = uv2.y() - uv0.y();

            float f = 1.0f/(x1*y2 - x2*y1);
            QVector3D t = (e1*y2 - e2*y1)*f;
            QVector3D b = (e2*x1 - e1*x2)*f;

            tangent[idx0] += t;
            tangent[idx1] += t;
            tangent[idx2] += t;
            bitangent[idx0] += b;
            bitangent[idx1] += b;
            bitangent[idx2] += b;
        }

        for(unsigned int i = 0; i < positions.size(); ++i) {
            QVector3D t = tangent[i];
            QVector3D b = bitangent[i];
            QVector3D n = normals[i];

            tangent[i] = (t - n*QVector3D::dotProduct(n, t)).normalized();
            if(QVector3D::dotProduct(QVector3D::crossProduct(t, b), n) <= 0.0f) {
                tangent[i].setX(-tangent[i].x());
            }
        }

        std::vector<float> data;
        for (int i = 0; i < positions.size(); ++i)
        {
            data.push_back(positions[i].x());
            data.push_back(positions[i].y());
            data.push_back(positions[i].z());
            if (normals.size() > 0)
            {
                data.push_back(normals[i].x());
                data.push_back(normals[i].y());
                data.push_back(normals[i].z());
            }
            if (uv.size() > 0)
            {
                data.push_back(uv[i].x());
                data.push_back(uv[i].y());
            }
            if(tangent.size() > 0) {
                data.push_back(tangent[i].x());
                data.push_back(tangent[i].y());
                data.push_back(tangent[i].z());
            }
        }
        glBindVertexArray(sphereVAO);
        glBindBuffer(GL_ARRAY_BUFFER, vbo);
        glBufferData(GL_ARRAY_BUFFER, data.size() * sizeof(float), &data[0], GL_STATIC_DRAW);
        glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, ebo);
        glBufferData(GL_ELEMENT_ARRAY_BUFFER, indices.size() * sizeof(unsigned int), &indices[0], GL_STATIC_DRAW);
        float stride = (3 + 2 + 3 + 3) * sizeof(float);
        glEnableVertexAttribArray(0);
        glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, stride, (void*)0);
        glEnableVertexAttribArray(1);
        glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, stride, (void*)(3 * sizeof(float)));
        glEnableVertexAttribArray(2);
        glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, stride, (void*)(6 * sizeof(float)));
        glEnableVertexAttribArray(3);
        glVertexAttribPointer(3, 3, GL_FLOAT, GL_FALSE, stride, (void*)(8 * sizeof(float)));
   }
    glBindVertexArray(sphereVAO);
    glDrawElements(GL_TRIANGLES, indexCount, GL_UNSIGNED_INT, 0);
    glBindVertexArray(0);
}

void Preview3DRenderer::renderPlane() {
    if (!planeVAO)
    {
        unsigned int quadVBO;

        float quadVertices[] = {
            -1.0f,  1.0f, 0.0f, 0.0f, 0.0f, 1.0f, 1.0f, 0.0f, -2.0f, 0.0f, 0.0f,
            -1.0f, -1.0f, 0.0f, 0.0f, 0.0f, 1.0f, 1.0f, 1.0f, -2.0f, 0.0f, 0.0f,
             1.0f,  1.0f, 0.0f, 0.0f, 0.0f, 1.0f, 0.0f, 0.0f, -2.0f, 0.0f, 0.0f,
             1.0f, -1.0f, 0.0f, 0.0f, 0.0f, 1.0f, 0.0f, 1.0f, -2.0f, 0.0f, 0.0f
        };

        glGenVertexArrays(1, &planeVAO);
        glGenBuffers(1, &quadVBO);
        glBindVertexArray(planeVAO);
        glBindBuffer(GL_ARRAY_BUFFER, quadVBO);
        glBufferData(GL_ARRAY_BUFFER, sizeof(quadVertices), &quadVertices, GL_STATIC_DRAW);
        glEnableVertexAttribArray(0);
        glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 11 * sizeof(float), nullptr);
        glEnableVertexAttribArray(1);
        glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 11 * sizeof(float), (void*)(3 * sizeof(float)));
        glEnableVertexAttribArray(2);
        glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, 11 * sizeof(float), (void*)(6 * sizeof(float)));
        glEnableVertexAttribArray(3);
        glVertexAttribPointer(3, 3, GL_FLOAT, GL_FALSE, 11 * sizeof(float), (void*)(8 * sizeof(float)));
        glBindBuffer(GL_ARRAY_BUFFER, 0);
    }
    glBindVertexArray(planeVAO);
    glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);
    glBindVertexArray(0);
}

void Preview3DRenderer::renderScene() {
    pbrShader->bind();

    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_CUBE_MAP, irradianceMap);
    glActiveTexture(GL_TEXTURE1);
    glBindTexture(GL_TEXTURE_CUBE_MAP, prefilterMap);
    glActiveTexture(GL_TEXTURE2);
    glBindTexture(GL_TEXTURE_2D, brdfLUTTexture);
    glActiveTexture(GL_TEXTURE3);
    glBindTexture(GL_TEXTURE_2D, albedoTexture);
    glActiveTexture(GL_TEXTURE4);
    glBindTexture(GL_TEXTURE_2D, normalTexture);
    glActiveTexture(GL_TEXTURE5);
    glBindTexture(GL_TEXTURE_2D, metalTexture);
    glActiveTexture(GL_TEXTURE6);
    glBindTexture(GL_TEXTURE_2D, roughTexture);
    glActiveTexture(GL_TEXTURE7);
    glBindTexture(GL_TEXTURE_2D, heightTexture);
    glActiveTexture(GL_TEXTURE8);
    glBindTexture(GL_TEXTURE_2D, emissionTexture);
    switch (primitive) {
        case 0: default:
            glEnable(GL_CULL_FACE);
            renderSphere();
            glDisable(GL_CULL_FACE);
            break;
        case 1:
            glEnable(GL_CULL_FACE);
            renderCube();
            glDisable(GL_CULL_FACE);
            break;
        case 2:
            renderPlane();
            break;
    }

    pbrShader->release();
    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, 0);
}

void Preview3DRenderer::renderForBloom() {
    glBindFramebuffer(GL_FRAMEBUFFER, multisampleFBO);

    glViewport(0, 0, wWidth, wHeight);
    glEnable(GL_MULTISAMPLE);
    glEnable(GL_DEPTH_TEST);
    glDepthFunc(GL_LEQUAL);
    glEnable(GL_TEXTURE_CUBE_MAP_SEAMLESS);
    glClearColor(0.039f, 0.042f, 0.045f, 1.0f);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    renderScene();

    glDisable(GL_DEPTH_TEST);
    glBindFramebuffer(GL_FRAMEBUFFER, 0);

    glBindFramebuffer(GL_READ_FRAMEBUFFER, multisampleFBO);
    glBindFramebuffer(GL_DRAW_FRAMEBUFFER, screenFBO);
    glBlitFramebuffer(0, 0, wWidth, wHeight, 0, 0, wWidth, wHeight, GL_COLOR_BUFFER_BIT, GL_NEAREST);
    glBindFramebuffer(GL_READ_FRAMEBUFFER, 0);
    glBindFramebuffer(GL_DRAW_FRAMEBUFFER, 0);

    glBindFramebuffer(GL_FRAMEBUFFER, hdrFBO);
    glViewport(0, 0, wWidth, wHeight);
    glClearColor(0.0f, 0.0f, 0.0f, 0.0f);
    glClear(GL_COLOR_BUFFER_BIT);
    brightShader->bind();
    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, screenTexture);
    renderQuad();
    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, 0);
    brightShader->release();
    glBindFramebuffer(GL_FRAMEBUFFER, 0);
}

void Preview3DRenderer::brightnessBlur(){
    bool first_iteration = true;
    QVector2D dir = QVector2D(1, 0);
    int amount = 50;
    glDisable(GL_DEPTH_TEST);
    unsigned int width = wWidth*(1.0f/bloomRadius);
    unsigned int height = wHeight*(1.0f/bloomRadius);
    glBindTexture(GL_TEXTURE_2D, pingpongBuffer[0]);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA16F, width, height, 0, GL_RGBA, GL_FLOAT, nullptr);
    glBindTexture(GL_TEXTURE_2D, pingpongBuffer[1]);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA16F, width, height, 0, GL_RGBA, GL_FLOAT, nullptr);

    for (unsigned int i = 0; i < amount; i++)
    {
        glBindFramebuffer(GL_FRAMEBUFFER, pingpongFBO[i%2]);

        glActiveTexture(GL_TEXTURE0);
        glBindTexture(
            GL_TEXTURE_2D, first_iteration ? brightTexture : pingpongBuffer[1 - (i%2)]
        );

        glViewport(0, 0, width, height);
        glClearColor(0.0f, 0.0f, 0.0f, 0.0f);
        glClear(GL_COLOR_BUFFER_BIT);
        blurShader->bind();
        blurShader->setUniformValue(blurShader->uniformLocation("direction"), dir);
        blurShader->setUniformValue(blurShader->uniformLocation("zoom"), zoom);
        dir = QVector2D(1, 1) - dir;
        blurShader->setUniformValue(blurShader->uniformLocation("resolution"), QVector2D(width, height));
        renderQuad();
        glActiveTexture(GL_TEXTURE0);
        glBindTexture(GL_TEXTURE_2D, 0);
        blurShader->release();
        glBindVertexArray(0);
        if (first_iteration)
            first_iteration = false;
    }

    glBindFramebuffer(GL_FRAMEBUFFER, 0);

}

void Preview3DRenderer::updateMatrix() {
    projection = QMatrix4x4();
    projection.perspective(zoom, (float)wWidth/(float)wHeight, 0.1f, 38.0f);
    view = QMatrix4x4();
    view.lookAt(positionV, positionV + QVector3D(0, 0, -1), QVector3D(0, -1, 0));
    model = QMatrix4x4();
    model.translate(0.0f, 0.0f, 0.0f);
    model.rotate(rotQuat);
    pbrShader->bind();
    pbrShader->setUniformValue(pbrShader->uniformLocation("projection"), projection);
    pbrShader->setUniformValue(pbrShader->uniformLocation("view"), view);
    pbrShader->setUniformValue(pbrShader->uniformLocation("model"), model);
}

void Preview3DRenderer::updateTexResolution() {
    glBindTexture(GL_TEXTURE_2D, albedoTexture);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, m_texResolution.x(), m_texResolution.y(), 0, GL_RGBA, GL_UNSIGNED_BYTE, nullptr);
    glBindTexture(GL_TEXTURE_2D, roughTexture);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, m_texResolution.x(), m_texResolution.y(), 0, GL_RGBA, GL_UNSIGNED_BYTE, nullptr);
    glBindTexture(GL_TEXTURE_2D, metalTexture);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, m_texResolution.x(), m_texResolution.y(), 0, GL_RGBA, GL_UNSIGNED_BYTE, nullptr);
    glBindTexture(GL_TEXTURE_2D, normalTexture);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, m_texResolution.x(), m_texResolution.y(), 0, GL_RGBA, GL_UNSIGNED_BYTE, nullptr);
    glBindTexture(GL_TEXTURE_2D, heightTexture);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, m_texResolution.x(), m_texResolution.y(), 0, GL_RGBA, GL_UNSIGNED_BYTE, nullptr);
    glBindTexture(GL_TEXTURE_2D, 0);
}

void Preview3DRenderer::updateOutputsTexture(unsigned int &dst, const unsigned int &src) {
    glBindFramebuffer(GL_FRAMEBUFFER, outputFBO);
    glBindTexture(GL_TEXTURE_2D, dst);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, m_texResolution.x(), m_texResolution.y(), 0, GL_RGBA, GL_UNSIGNED_BYTE, nullptr);
    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, dst, 0);
    glDisable(GL_DEPTH_TEST);
    glViewport(0, 0, m_texResolution.x(), m_texResolution.y());
    glClearColor(0.0f, 0.0f, 0.0f, 0.0f);
    glClear(GL_COLOR_BUFFER_BIT);
    textureShader->bind();
    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, src);
    renderQuad();
    glBindTexture(GL_TEXTURE_2D, 0);
    textureShader->release();
    glBindFramebuffer(GL_FRAMEBUFFER, 0);

    glBindTexture(GL_TEXTURE_2D, dst);
    glGenerateMipmap(GL_TEXTURE_2D);
    glBindTexture(GL_TEXTURE_2D, 0);
    glFlush();
    glFinish();
}

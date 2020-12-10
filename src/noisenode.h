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

#ifndef NOISENODE_H
#define NOISENODE_H
#include "node.h"
#include "noise.h"

struct NoiseParams {
    float persistence = 0.5f;
    float amplitude = 1.0f;
    int scaleX = 1;
    int scaleY = 1;
    int scale = 5;
    int layers = 8;
};

class NoiseNode: public Node
{
    Q_OBJECT
public:
    NoiseNode(QQuickItem *parent = nullptr, QVector2D resolution = QVector2D(1024, 1024), NoiseParams perlin = NoiseParams(), NoiseParams simple = NoiseParams{0.5f, 1.0f, 1, 1, 20, 8}, QString noiseType = "noisePerlin");
    ~NoiseNode();
    QString noiseType();    
    void setNoiseType(QString type);
    NoiseParams perlinParams();
    NoiseParams simpleParams();
    int noiseScale();
    void setNoiseScale(int scale);
    int scaleX();
    void setScaleX(int scale);
    int scaleY();
    void setScaleY(int scale);
    int layers();
    void setLayers(int num);
    float persistence();
    void setPersistence(float value);
    float amplitude();
    void setAmplitude(float value);
    unsigned int &getPreviewTexture();
    void operation();
    void serialize(QJsonObject &json) const;
    void deserialize(const QJsonObject &json);
signals:
    void noiseTypeChanged(QString type);
    void noiseScaleChanged(float scale);
    void scaleXChanged(float scale);
    void scaleYChanged(float scale);
    void layersChanged(int layers);
    void persistenceChanged(float persistence);
    void amplitudeChanged(float amplitude);
public slots:
    void updateNoiseType(QString type);
    void updateNoiseScale(qreal scale);
    void updateScaleX(qreal scale);
    void updateScaleY(qreal scale);
    void setOutput();
    void updateLayers(int layers);
    void updatePersistence(qreal value);
    void updateAmplitude(qreal value);
    void updatePrev(bool sel);
    void previewGenerated();
    void updateScale(float scale);
private:
    NoiseObject *preview = nullptr;
    QString m_noiseType = "perlinNoise";
    NoiseParams perlinNoise;
    NoiseParams simpleNoise;
};

#endif // NOISENODE_H

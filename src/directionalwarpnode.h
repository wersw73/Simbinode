#ifndef DIRECTIONALWARPNODE_H
#define DIRECTIONALWARPNODE_H

#include "node.h"
#include "directionalwarp.h"

class DirectionalWarpNode: public Node
{
    Q_OBJECT
public:
    DirectionalWarpNode(QQuickItem *parent = nullptr, QVector2D resolution = QVector2D(1024, 1024), GLint bpc = GL_RGBA8, float intensity = 0.1f, int angle = 0);
    ~DirectionalWarpNode();
    void operation() override;
    unsigned int &getPreviewTexture() override;
    void saveTexture(QString fileName) override;
    DirectionalWarpNode *clone() override;
    void serialize(QJsonObject &json) const override;
    void deserialize(const QJsonObject &json, QHash<QUuid, Socket*> &hash) override;
    float intensity();
    void setIntnsity(float intensity);
    int angle();
    void setAngle(int angle);
signals:
    void intensityChanged(float intensity);
    void angleChanged(int angle);
public slots:
    void updateIntensity(qreal intensity);
    void updateAngle(int angle);
    void setOutput();
private:
    DirectionalWarpObject *preview;
    float m_intensity = 0.1f;
    int m_angle = 0;
};

#endif // DIRECTIONALWARPNODE_H

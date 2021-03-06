#ifndef GRAYSCALENODE_H
#define GRAYSCALENODE_H

#include "node.h"
#include "grayscale.h"

class GrayscaleNode: public Node
{
    Q_OBJECT
public:
    GrayscaleNode(QQuickItem *parent = nullptr, QVector2D resolution = QVector2D(1024, 1024), GLint bpc = GL_RGBA8);
    ~GrayscaleNode();
    void operation() override;
    unsigned int &getPreviewTexture() override;
    void saveTexture(QString fileName) override;
    GrayscaleNode *clone() override;
    void serialize(QJsonObject &json) const override;
    void deserialize(const QJsonObject &json, QHash<QUuid, Socket*> &hash) override;
public slots:
    void previewGenerated();
    void setOutput();
private:
    GrayscaleObject *preview;
};

#endif // GRAYSCALENODE_H

#ifndef QUADRILATERALITEM_H
#define QUADRILATERALITEM_H

#include <QGraphicsItemGroup>
#include <QGraphicsLineItem>
#include <QGraphicsRectItem>
#include <QGraphicsScene>
#include <QObject>
#include <opencv2/core.hpp>
#include <vector>

// Forward declaration
class CornerItem;

// QuadrilateralItem class definition
class QuadrilateralItem : public QObject, public QGraphicsItemGroup {
    Q_OBJECT
public:
    explicit QuadrilateralItem(const std::vector<cv::Point> &points, QGraphicsScene *scene, QObject *parent = nullptr);
    void updateLines();

protected:
private:
    std::vector<CornerItem *> corners;
    std::vector<QGraphicsLineItem *> lines;
    void deleteQuad();
};

// CornerItem class definition
class CornerItem : public QObject, public QGraphicsRectItem {
    Q_OBJECT
public:
    explicit CornerItem(const cv::Point &position, QGraphicsScene *scene, QObject *parent = nullptr);

signals:
    void positionChanged();
    void deletePressed();

protected:
    void mouseMoveEvent(QGraphicsSceneMouseEvent *event) override;
    void mousePressEvent(QGraphicsSceneMouseEvent *event) override;
};

#endif // QUADRILATERALITEM_H

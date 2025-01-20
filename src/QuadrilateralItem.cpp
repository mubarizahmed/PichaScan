#include "QuadrilateralItem.h"
#include <QCursor>
#include <QGraphicsSceneMouseEvent>
#include <QPen>
#include <stdexcept>

// QuadrilateralItem constructor
QuadrilateralItem::QuadrilateralItem(const std::vector<cv::Point> &points, QGraphicsScene *scene, QObject *parent)
    : QObject(parent), QGraphicsItemGroup() {
    // Ensure we have exactly 4 points
    if (points.size() != 4) {
        throw std::invalid_argument("QuadrilateralItem requires exactly 4 points.");
    }

    QGraphicsScene *parentScene = this->scene();

    // Create corners and lines
    for (const auto &point : points) {
        auto *corner = new CornerItem(point, scene, this);
        corners.push_back(corner);

        connect(corner, &CornerItem::positionChanged, this, &QuadrilateralItem::updateLines);
        connect(corner, &CornerItem::deletePressed, this, &QuadrilateralItem::deleteQuad);
    }

    for (int i = 0; i < 4; i++) {
        auto *line = new QGraphicsLineItem(this);
        line->setPen(QPen(Qt::red, scene->width() / 160));
        lines.push_back(line);
    }
    // Initial line positions
    updateLines();
    scene->addItem(this);
}

// Update lines based on corner positions
void QuadrilateralItem::updateLines() {
    for (int i = 0; i < 4; i++) {
        int nextIndex = (i + 1) % 4;
        lines[i]->setLine(QLineF(corners[i]->pos(), corners[nextIndex]->pos()));
    }
}

void QuadrilateralItem::deleteQuad() {
    // this->scene()->removeItem(this);
    // scene()->removeItem(this);
    delete this;
}

// CornerItem constructor
CornerItem::CornerItem(const cv::Point &position, QGraphicsScene *scene, QObject *parent)
    : QObject(parent), QGraphicsRectItem(-5, -5, 10, 10) {
    // set height and width to 50
    qreal side = scene->width() / 80;
    setRect(-side / 2, -side / 2, side, side);

    setPos(position.x, position.y);

    setBrush(Qt::black);
    setFlag(QGraphicsItem::ItemIsMovable, true);
    setFlag(QGraphicsItem::ItemIsSelectable, true);
    setZValue(10);
    setCursor(Qt::PointingHandCursor);
    scene->addItem(this);
}

// Emit positionChanged signal on mouse release
void CornerItem::mouseMoveEvent(QGraphicsSceneMouseEvent *event) {
    QGraphicsRectItem::mouseMoveEvent(event);
    emit positionChanged();
}

void CornerItem::mousePressEvent(QGraphicsSceneMouseEvent *event) {
    // delete object if right-click
    if (event->button() == Qt::RightButton) {
        emit deletePressed();
    }
}
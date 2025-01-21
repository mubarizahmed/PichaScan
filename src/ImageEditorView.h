#ifndef IMAGE_EDITOR_VIEW_H
#define IMAGE_EDITOR_VIEW_H

#include <QGraphicsView>
#include <QMouseEvent> // for QMouseEvent
#include <QPushButton>
#include <QWheelEvent>
#include <QWidget> // for QWidget if needed
#include <opencv2/core.hpp>
#include "QuadrilateralItem.h"

class ImageEditorView : public QGraphicsView {
    Q_OBJECT

public:
    explicit ImageEditorView(QWidget *parent);
    void positionButtons();
    void addQuadrilateral(std::vector<cv::Point> points);
    void addEmptyQuadrilateral();
    void deleteQuad(QuadrilateralItem *q);
    void updateQuads();

protected:
    void wheelEvent(QWheelEvent *event) override;
    // void mousePressEvent(QMouseEvent *event) override;
    // void mouseReleaseEvent(QMouseEvent *event) override;
    void resizeEvent(QResizeEvent *event) override;

signals:
    void quadrilateralsChanged(std::vector<std::vector<cv::Point>> quads);
    void scanRotated(int angle);

private slots:
    void rotateSceneLeft();
    void rotateSceneRight();

private:
    QWidget *buttonOverlay;
    QPushButton *rotateLeftButton;
    QPushButton *rotateRightButton;
    QPushButton *addQuadrilateralButton;
};

#endif // IMAGE_EDITOR_VIEW_H

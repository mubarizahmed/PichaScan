#include "ImageEditorView.h"
#include "QuadrilateralItem.h"

#include <QVBoxLayout>

ImageEditorView::ImageEditorView(QWidget *parent = nullptr)
    : QGraphicsView(parent),
      buttonOverlay(new QWidget(this)),
      rotateLeftButton(new QPushButton("⟲", buttonOverlay)),
      rotateRightButton(new QPushButton("⟳", buttonOverlay)),
      addQuadrilateralButton(new QPushButton("+", buttonOverlay)) {
    setRenderHints(QPainter::Antialiasing | QPainter::SmoothPixmapTransform);
    setDragMode(QGraphicsView::ScrollHandDrag);
    setInteractive(true); // allows item interaction

        // Button overlay layout
    auto *layout = new QVBoxLayout(buttonOverlay);
    layout->setSpacing(5);
    layout->setContentsMargins(0, 0, 0, 0);
    layout->addWidget(rotateLeftButton);
    layout->addWidget(rotateRightButton);
    layout->addWidget(addQuadrilateralButton);

    // Style the buttons
    rotateLeftButton->setFixedSize(30, 30);
    rotateRightButton->setFixedSize(30, 30);
    addQuadrilateralButton->setFixedSize(30, 30);

    buttonOverlay->setAttribute(Qt::WA_TransparentForMouseEvents, false);
    buttonOverlay->setStyleSheet("background: rgba(0, 0, 0, 0.8); border-radius: 5px;");
    buttonOverlay->move(width() - buttonOverlay->width() - 10, 10); // Initial position

    // Connect button signals to slots
    connect(rotateLeftButton, &QPushButton::clicked, this, &ImageEditorView::rotateSceneLeft);
    connect(rotateRightButton, &QPushButton::clicked, this, &ImageEditorView::rotateSceneRight);
    connect(addQuadrilateralButton, &QPushButton::clicked, this, &ImageEditorView::addQuadrilateral);

}

void ImageEditorView::wheelEvent(QWheelEvent *event) {
    // Zoom factor
    constexpr double scaleFactor = 1.15;
    if (event->angleDelta().y() > 0) {
        // zoom in
        scale(scaleFactor, scaleFactor);
    } else {
        // zoom out
        scale(1.0 / scaleFactor, 1.0 / scaleFactor);
    }
    event->accept();
}

// void ImageEditorView::mousePressEvent(QMouseEvent *event) {
//     if (event->button() == Qt::MiddleButton) {
//         setDragMode(QGraphicsView::ScrollHandDrag);
//         // Manually send the event to the base so it starts the drag
//         QGraphicsView::mousePressEvent(event);
//     } else {
//         QGraphicsView::mousePressEvent(event);
//     }
// }

// void ImageEditorView::mouseReleaseEvent(QMouseEvent *event) {
//     if (event->button() == Qt::MiddleButton) {
//         setDragMode(QGraphicsView::NoDrag);
//         QGraphicsView::mouseReleaseEvent(event);
//     } else {
//         QGraphicsView::mouseReleaseEvent(event);
//     }
// }

void ImageEditorView::rotateSceneLeft() {
    rotate(-90); // Rotate the scene 90 degrees counter-clockwise
}

void ImageEditorView::rotateSceneRight() {
    rotate(90); // Rotate the scene 90 degrees clockwise
}

void ImageEditorView::addQuadrilateral() {
    // find the middle of the scene and half the width and generate 4 points with a 4:3 aspect ratio
    double width = this->scene()->width();

    cv::Point2d center(width / 2, this->scene()->height() / 2);

    // generate vector of 4 points
    std::vector<cv::Point> points = {
        cv::Point(center.x - width / 6, center.y - width / 8),
        cv::Point(center.x + width / 6, center.y - width / 8),
        cv::Point(center.x + width / 6, center.y + width / 8),
        cv::Point(center.x - width / 6, center.y + width / 8)};


    auto *quad = new QuadrilateralItem(points, this->scene(), this->scene());
}

void ImageEditorView::resizeEvent(QResizeEvent *event) {
    QGraphicsView::resizeEvent(event);
    // Reposition overlay on resize
    buttonOverlay->move(width() - buttonOverlay->width() - 10, 10);
}
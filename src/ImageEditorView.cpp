#include "ImageEditorView.h"

ImageEditorView::ImageEditorView(QWidget *parent = nullptr)
    : QGraphicsView(parent) {
    setRenderHints(QPainter::Antialiasing | QPainter::SmoothPixmapTransform);
    setDragMode(QGraphicsView::ScrollHandDrag);
    setInteractive(true); // allows item interaction
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

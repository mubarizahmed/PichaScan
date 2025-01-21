#include "ImageEditorView.h"
// #include "QuadrilateralItem.h"

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
    connect(addQuadrilateralButton, &QPushButton::clicked, this, &ImageEditorView::addEmptyQuadrilateral);
}

void ImageEditorView::wheelEvent(QWheelEvent *event) {
    // Zoom factor
    constexpr double scaleFactor = 1.15;

    // Get the position of the mouse in scene coordinates
    QPointF scenePosBefore = mapToScene(event->position().toPoint());
    setTransformationAnchor(QGraphicsView::AnchorUnderMouse);
    qDebug() << "ScenePosBefore:" << scenePosBefore;
    // Determine the zoom direction
    double factor = (event->angleDelta().y() > 0) ? scaleFactor : (1.0 / scaleFactor);

    // Scale the scene (via the view's transformation matrix)
    scale(factor, factor);

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
    emit scanRotated(-90);
}

void ImageEditorView::rotateSceneRight() {
    rotate(90); // Rotate the scene 90 degrees clockwise
    emit scanRotated(90);
}

void ImageEditorView::addEmptyQuadrilateral() {
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

    connect(quad, &QuadrilateralItem::positionChanged, this, &ImageEditorView::updateQuads);
    connect(quad, &QuadrilateralItem::deletePressed, this, &ImageEditorView::deleteQuad);

    updateQuads();
}



void ImageEditorView::addQuadrilateral(std::vector<cv::Point> points) {

    auto *quad = new QuadrilateralItem(points, this->scene(), this->scene());

    connect(quad, &QuadrilateralItem::positionChanged, this, &ImageEditorView::updateQuads);
    connect(quad, &QuadrilateralItem::deletePressed, this, &ImageEditorView::deleteQuad);
}

void ImageEditorView::deleteQuad(QuadrilateralItem *q) {
    delete q;
    updateQuads();
}

void ImageEditorView::resizeEvent(QResizeEvent *event) {
    QGraphicsView::resizeEvent(event);
    // Reposition overlay on resize
    positionButtons();
}

void ImageEditorView::positionButtons() {
    buttonOverlay->move(width() - buttonOverlay->width() - 10, 10);
}

void ImageEditorView::updateQuads() {
    // Update all quadrilaterals
    std::vector<std::vector<cv::Point>> quads;
    for (auto item : scene()->items()) {
        if (auto *quad = qgraphicsitem_cast<QuadrilateralItem *>(item)) {
            quads.push_back(quad->getCorners());
        }
    }

    emit quadrilateralsChanged(quads);
}
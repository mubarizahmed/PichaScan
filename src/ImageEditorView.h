#ifndef IMAGE_EDITOR_VIEW_H
#define IMAGE_EDITOR_VIEW_H

#include <QGraphicsView>
#include <QMouseEvent> // for QMouseEvent
#include <QWheelEvent>
#include <QWidget> // for QWidget if needed
#include <QPushButton>
#include <opencv2/core.hpp>

class ImageEditorView : public QGraphicsView {
    Q_OBJECT

public:
    explicit ImageEditorView(QWidget *parent);

protected:
    void wheelEvent(QWheelEvent *event) override;
    // void mousePressEvent(QMouseEvent *event) override;
    // void mouseReleaseEvent(QMouseEvent *event) override;
    void resizeEvent(QResizeEvent *event) override;

private slots:
    void rotateSceneLeft();
    void rotateSceneRight();
    void addQuadrilateral();

private:
    QWidget *buttonOverlay;
    QPushButton *rotateLeftButton;
    QPushButton *rotateRightButton;
    QPushButton *addQuadrilateralButton;
};

#endif // IMAGE_EDITOR_VIEW_H

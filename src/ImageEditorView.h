#ifndef IMAGE_EDITOR_VIEW_H
#define IMAGE_EDITOR_VIEW_H

#include <QGraphicsView>
#include <QMouseEvent> // for QMouseEvent
#include <QWheelEvent>
#include <QWidget> // for QWidget if needed

class ImageEditorView : public QGraphicsView {
    Q_OBJECT

public:
    explicit ImageEditorView(QWidget *parent);

protected:
    void wheelEvent(QWheelEvent *event) override;
    // void mousePressEvent(QMouseEvent *event) override;
    // void mouseReleaseEvent(QMouseEvent *event) override;
};

#endif // IMAGE_EDITOR_VIEW_H

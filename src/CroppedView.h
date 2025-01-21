#ifndef CUSTOMLISTVIEW_H
#define CUSTOMLISTVIEW_H

#include <QHBoxLayout>
#include <QLabel>
#include <QListView>
#include <QListWidget>
#include <QPixmap>
#include <QPushButton>
#include <QVBoxLayout>
#include <QWidget>

class CroppedViewItem : public QWidget {
    Q_OBJECT

public:
    explicit CroppedViewItem(const QPixmap &pixmap, QWidget *parent = nullptr);
    void manualResize(int viewHeight);

protected:
    void enterEvent(QEnterEvent *event) override;
    void leaveEvent(QEvent *event) override;
    void resizeEvent(QResizeEvent *event);

signals:
    void rotateLeft();
    void rotateRight();

private:
    QPixmap originalPixmap;
    QLabel *imageLabel;
    QPushButton *rotateLeftButton;
    QPushButton *rotateRightButton;
};

class CroppedView : public QListWidget {
    Q_OBJECT

public:
    explicit CroppedView(QWidget *parent = nullptr);
    void addImageItem(const QPixmap &pixmap, int index);
    void manualResize();

signals:
    void viewItemRotated(int index, int angle);

protected:
    void resizeEvent(QResizeEvent *event) override;

private:
    int itemWidth;
    int itemHeight;
    void updateItemSizes();
};

#endif // CUSTOMLISTVIEW_H
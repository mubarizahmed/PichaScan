
#include "CroppedView.h"
#include <QDebug>
#include <QListWidgetItem>
#include <QStandardItem>

// --- CroppedViewItem Implementation ---
CroppedViewItem::CroppedViewItem(const QPixmap &pixmap, QWidget *parent)
    : QWidget(parent) {
    originalPixmap = pixmap;

    // Create the image label
    imageLabel = new QLabel(this);
    imageLabel->setPixmap(pixmap);
    imageLabel->setAlignment(Qt::AlignCenter);
    imageLabel->setSizePolicy(QSizePolicy::MinimumExpanding, QSizePolicy::MinimumExpanding);

    // Create the rotate buttons
    rotateLeftButton = new QPushButton("⟲", this);
    rotateRightButton = new QPushButton("⟳", this);

    // Style the buttons
    rotateLeftButton->setFixedSize(30, 30);
    rotateRightButton->setFixedSize(30, 30);
    rotateLeftButton->setVisible(false);
    rotateRightButton->setVisible(false);

    // Layout for buttons
    QHBoxLayout *buttonLayout = new QHBoxLayout();
    buttonLayout->addWidget(rotateLeftButton);
    buttonLayout->addWidget(rotateRightButton);
    buttonLayout->setAlignment(Qt::AlignHCenter);

    // Main layout
    QVBoxLayout *mainLayout = new QVBoxLayout(this);
    mainLayout->addWidget(imageLabel);
    mainLayout->addLayout(buttonLayout);
    mainLayout->setSpacing(5);
    mainLayout->setContentsMargins(0, 0, 0, 0);

    // Ensure widget expands to fill the parent
    setSizePolicy(QSizePolicy::MinimumExpanding, QSizePolicy::MinimumExpanding);

    // manualResize();

    // Connect buttons
    connect(rotateLeftButton, &QPushButton::clicked, this, &CroppedViewItem::rotateLeft);
    connect(rotateRightButton, &QPushButton::clicked, this, &CroppedViewItem::rotateRight);
}

void CroppedViewItem::resizeEvent(QResizeEvent *event) {
    qDebug() << "Item Resize Event w:" << width() << " h:" << height();
    // if (!originalPixmap.isNull()) {
    //     QPixmap scaledPixmap = originalPixmap.scaled(
    //         QSize(imageLabel->width() * 100, imageLabel->height()), Qt::KeepAspectRatio, Qt::SmoothTransformation);

    //     imageLabel->setMinimumWidth(scaledPixmap.width());
    //     imageLabel->setPixmap(scaledPixmap);
    // }
    // QWidget::resizeEvent(event);
}

void CroppedViewItem::manualResize(int viewHeight) {
    qDebug() << "Item Manual Resize w:" << width() << " h:" << height() << "viewH:" << viewHeight;
    if (!originalPixmap.isNull()) {
        QPixmap scaledPixmap = originalPixmap.scaled(
            QSize(imageLabel->width() * 100, viewHeight), Qt::KeepAspectRatio, Qt::SmoothTransformation);
        imageLabel->setFixedWidth(scaledPixmap.width());
        // imageLabel->setMinimumHeight(scaledPixmap.height());
        qDebug() << "Scaled Width:" << scaledPixmap.width() << " Height:" << scaledPixmap.height();
        setFixedWidth(scaledPixmap.width());
        // setMinimumHeight(scaledPixmap.height());
        imageLabel->setPixmap(scaledPixmap);
    }
}

void CroppedViewItem::enterEvent(QEnterEvent *event) {
    rotateLeftButton->setVisible(true);
    rotateRightButton->setVisible(true);
    QWidget::enterEvent(event);
}

void CroppedViewItem::leaveEvent(QEvent *event) {
    rotateLeftButton->setVisible(false);
    rotateRightButton->setVisible(false);
    QWidget::leaveEvent(event);
}

// --- CroppedView Implementation ---
CroppedView::CroppedView(QWidget *parent)
    : QListWidget(parent), itemWidth(50), itemHeight(50) {
    setFlow(QListWidget::LeftToRight);
    setResizeMode(QListWidget::Adjust);
    // setUniformItemSizes(true);
    // setGridSize(QSize(itemWidth, itemHeight));
    itemHeight = height();

    setSpacing(5);
}

void CroppedView::addImageItem(const QPixmap &pixmap, int index) {
    QListWidgetItem *item = new QListWidgetItem();
    item->setSizeHint(QSize(itemWidth, itemHeight));

    CroppedViewItem *viewItem = new CroppedViewItem(pixmap, this);
    connect(viewItem, &CroppedViewItem::rotateLeft, [this, index]() {
        qDebug() << "Rotate Left:" << index;
        emit viewItemRotated(index, -90);
    });
    connect(viewItem, &CroppedViewItem::rotateRight, [this, index]() {
        qDebug() << "Rotate Right:" << index;
        emit viewItemRotated(index, 90);
    });
    addItem(item);
    setItemWidget(item, viewItem);
    setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
}

void CroppedView::resizeEvent(QResizeEvent *event) {
    qDebug() << "Resize Event w:" << width() << " h:" << height();
    QListWidget::resizeEvent(event);

    manualResize();
}

void CroppedView::manualResize() {

    int dynamicHeight = height() * 0.9; // Example: Divide the height into 2 rows

    for (int i = 0; i < count(); ++i) {
        QListWidgetItem *item = this->item(i);
        if (item) {
            // item->setSizeHint(QSize(dynamicWidth, dynamicHeight));

            QWidget *widget = this->itemWidget(item);
            if (auto croppedViewItem = qobject_cast<CroppedViewItem *>(widget)) {
                int newWidth = 0;
                croppedViewItem->manualResize(dynamicHeight); // Adjust child widget sizes
                newWidth = croppedViewItem->width();

                if (newWidth != 0) {
                    qDebug() << "New Width:" << newWidth;
                    item->setSizeHint(QSize(newWidth*1, height() * 0.9));
                }
            }
        }
    }
}

void CroppedView::updateItemSizes() {
    if (parentWidget()) {
        itemHeight = height();
        itemWidth = width();
        setGridSize(QSize(itemWidth, itemHeight));
    }
}

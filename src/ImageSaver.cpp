#include "ImageSaver.h"
#include <QDebug>
#include <QFileInfo>

bool ImageSaver::saveImage(const cv::Mat& image, const QString& filePath)
{
    if (image.empty()) {
        qWarning() << "Image is empty, cannot save to" << filePath;
        return false;
    }
    // We can use OpenCV's imwrite, or convert to QImage and use QImage::save
    bool res = cv::imwrite(filePath.toStdString(), image);

    if (!res) {
        qWarning() << "Failed to save image to" << filePath;
    } else {
        qDebug() << "Image saved to" << filePath;
    }
    return res;
}
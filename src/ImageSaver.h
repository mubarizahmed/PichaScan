#pragma once
#include <opencv2/opencv.hpp>
#include <QString>

class ImageSaver
{
public:
    bool saveImage(const cv::Mat& image, const QString& filePath, const QString& dateTimeString);

    // Potentially add methods to embed metadata
    // bool embedMetadata(const QString& filePath, ...);
};
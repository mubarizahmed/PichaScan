#pragma once
#include <opencv2/opencv.hpp>
#include <QString>

class ImageSaver
{
public:
    bool saveImage(const cv::Mat& image, const QString& filePath, const QString& dateTimeString, std::pair<double, double> imageLocation);

    static std::string toExifString(double d, bool bRational, bool bLat);
    // Potentially add methods to embed metadata
    // bool embedMetadata(const QString& filePath, ...);
};
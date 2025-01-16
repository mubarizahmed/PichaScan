#pragma once

#include <opencv2/opencv.hpp>
#include <vector>

class ScanProcessor
{
public:
    ScanProcessor() = default;

    // Returns each cropped photo as an individual Mat
    std::vector<cv::Mat> detectAndCropPhotos(const cv::Mat& scannedImage);
};

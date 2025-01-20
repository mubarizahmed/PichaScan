#pragma once

#include <opencv2/opencv.hpp>
#include <vector>

/**
 * Represents a single detected photo region:
 *  - The 4 corner points (approx polygon)
 *  - A bounding box
 *  - The cropped Mat itself
 */
struct DetectedRegion
{
    std::vector<cv::Point> corners;  // 4 corner points (clockwise or counterclockwise)
    cv::Rect boundingBox;            // The bounding rectangle
    cv::Mat cropped;                 // The cropped image data
};

/**
 * The result of running the detectAndCrop process:
 *  - annotated: a copy of the original scanned image with rectangles drawn
 *  - regions: a list of detected regions
 */
struct ScanResult
{
    cv::Mat annotated;
    std::vector<DetectedRegion> regions;
};

class ScanProcessor
{
public:
    ScanProcessor() = default;

    // Returns each cropped photo as an individual Mat
    ScanResult detectAndCropPhotos(const cv::Mat& scannedImage);
    std::vector<cv::Mat> cropImages(const cv::Mat &scannedImage, std::vector<std::vector<cv::Point>> quads);
};



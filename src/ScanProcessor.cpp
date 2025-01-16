#include "ScanProcessor.h"
#include <algorithm>

std::vector<cv::Mat> ScanProcessor::detectAndCropPhotos(const cv::Mat& scannedImage)
{
    std::vector<cv::Mat> results;

    if (scannedImage.empty()) {
        return results;
    }

    // 1. Convert to grayscale
    cv::Mat gray;
    if (scannedImage.channels() == 3) {
        cv::cvtColor(scannedImage, gray, cv::COLOR_BGR2GRAY);
    } else {
        gray = scannedImage.clone();
    }

    // 2. Threshold
    cv::Mat thresh;
    cv::threshold(gray, thresh, 128, 255, cv::THRESH_BINARY | cv::THRESH_OTSU);

    // 3. Find contours
    std::vector<std::vector<cv::Point>> contours;
    cv::findContours(thresh, contours, cv::RETR_EXTERNAL, cv::CHAIN_APPROX_SIMPLE);

    // 4. Filter for large rectangles (photos)
    for (const auto& contour : contours) {
        // Approximate polygon
        std::vector<cv::Point> approx;
        cv::approxPolyDP(contour, approx, 10, true);

        // Check if it's a quadrilateral
        if (approx.size() == 4) {
            // Potentially check area or bounding box size to exclude small/noise
            cv::Rect boundingRect = cv::boundingRect(approx);
            if (boundingRect.area() < 10000) {
                // skip small
                continue;
            }

            // 5. Crop
            // If we want a simple bounding rect crop:
            cv::Mat cropped = scannedImage(boundingRect).clone();
            results.push_back(cropped);

            // If we want perspective correction, we’d do a warpPerspective here
        }
    }

    return results;
}

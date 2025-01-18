#include "ScanProcessor.h"
#include <algorithm>

ScanResult ScanProcessor::detectAndCropPhotos(const cv::Mat& scannedImage)
{
    ScanResult result;

    // If input is empty, return empty result.
    if (scannedImage.empty()) {
        return result;
    }

    // Make a copy of the original image for annotation.
    // We'll draw the detected rectangles/polygons on this copy.
    result.annotated = scannedImage.clone();

    // 1. Convert to grayscale if needed
    cv::Mat gray;
    if (scannedImage.channels() == 3) {
        cv::cvtColor(scannedImage, gray, cv::COLOR_BGR2GRAY);
    } else {
        gray = scannedImage.clone();
    }

    // 2. Threshold (e.g. Otsu’s)
    cv::Mat thresh;
    cv::threshold(gray, thresh, 128, 255, cv::THRESH_BINARY | cv::THRESH_OTSU);

    // 3. Find contours
    std::vector<std::vector<cv::Point>> contours;
    cv::findContours(thresh, contours, cv::RETR_EXTERNAL, cv::CHAIN_APPROX_SIMPLE);

       // 1. Load the scanned image
    cv::Mat img = cv::imread("scan.jpg");
    if (img.empty()) {
        std::cerr << "Error: Could not open or find the image.\n";
    }

    // 2. Convert to grayscale and blur to reduce noise
    cv::Mat gray;
    cv::cvtColor(img, gray, cv::COLOR_BGR2GRAY);

    cv::Mat blurImg;
    cv::GaussianBlur(gray, blurImg, cv::Size(5, 5), 0);

    // 3. Use Canny edge detection
    cv::Mat edges;
    cv::Canny(blurImg, edges, 75, 200);

    // 4. Find contours (external only in this case)
    std::vector<std::vector<cv::Point>> contours;
    std::vector<cv::Vec4i> hierarchy;
    cv::findContours(edges, contours, hierarchy, cv::RETR_EXTERNAL, cv::CHAIN_APPROX_SIMPLE);

    // Filter out too-small contours by area
    double minArea = 10000.0;  // Adjust to match your expected doc sizes
    std::vector<std::vector<cv::Point>> candidateContours;
    for (size_t i = 0; i < contours.size(); i++) {
        double area = cv::contourArea(contours[i]);
        if (area > minArea) {
            candidateContours.push_back(contours[i]);
        }
    }

    // 5. Approximate each candidate contour and check if it’s a quadrilateral
    std::vector<std::vector<cv::Point>> docs;
    for (size_t i = 0; i < candidateContours.size(); i++) {
        double perimeter = cv::arcLength(candidateContours[i], true);
        std::vector<cv::Point> approx;
        cv::approxPolyDP(candidateContours[i], approx, 0.02 * perimeter, true);

        // If it has 4 corners, it's likely a rectangular document
        if (approx.size() == 4) {
            docs.push_back(approx);
        }
    }

    // 4. Filter for large rectangles (photos)
    for (const auto& contour : contours) {
        // Approximate polygon
        std::vector<cv::Point> approx;
        // The 10 here is a parameter controlling polygon approximation detail
        cv::approxPolyDP(contour, approx, 10, true);

        // Check if we got a quadrilateral
        if (approx.size() == 4) {
            // We can also check the area to exclude very small or noise
            double area = std::fabs(cv::contourArea(approx));
            if (area < 10000.0) {
                // skip small
                continue;
            }

            // Create a DetectedRegion struct
            DetectedRegion region;
            region.corners = approx;

            // Option 1: simple bounding box crop
            cv::Rect boundingRect = cv::boundingRect(approx);
            region.boundingBox = boundingRect;
            region.cropped = scannedImage(boundingRect).clone();

            // Draw the quadrilateral on the annotated image
            // (We could draw lines between approx[0]->approx[1], approx[1]->approx[2], etc.)
            cv::polylines(result.annotated, approx, /*isClosed=*/true, 
                          cv::Scalar(0, 0, 255), /*thickness=*/2);

            // Add this region to the result
            result.regions.push_back(std::move(region));
        }
    }

    return result;
}

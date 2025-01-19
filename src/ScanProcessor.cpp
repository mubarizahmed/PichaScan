#include "ScanProcessor.h"
#include <algorithm>

ScanResult ScanProcessor::detectAndCropPhotos(const cv::Mat& scannedImage)
{
    ScanResult result;

    // If input is empty, return empty result.
    if (scannedImage.empty()) {
        return result;
    }
    result.annotated = scannedImage.clone();

    // 1. Get saturation channel
    cv::Mat hsv; 
    cv::cvtColor(scannedImage, hsv, cv::COLOR_BGR2HSV);
    std::vector<cv::Mat> hsv_channels;
    cv::split(hsv, hsv_channels); // Split the HSV image into its 3 channels
    cv::Mat saturation = hsv_channels[1]; // Get the second channel (saturation)

    // 2. Threshhold
    cv::Mat thresh;
    cv::threshold(saturation, thresh, 5, 255, cv::THRESH_BINARY);

    // 3. Find contours
    std::vector<std::vector<cv::Point>> contours;
    cv::findContours(thresh, contours, cv::RETR_EXTERNAL, cv::CHAIN_APPROX_SIMPLE);

    // 4. Filter for large areas
    double imgArea = scannedImage.rows * scannedImage.cols;
    std::vector<std::vector<cv::Point>> largeContours;
    for (const auto& contour : contours) {
        double contourArea = cv::contourArea(contour);

        if (contourArea > (imgArea/8)) {
            largeContours.push_back(contour);
        }
    }

    // 5. Approximate each contour and check if it's a quadrilateral

    for (const auto& contour : largeContours) {
        double perimeter = cv::arcLength(contour, true);

        for (int eps=500; eps>0; eps--) {
            std::vector<cv::Point> approx;
            cv::approxPolyDP(contour, approx, 0.001 * eps * perimeter, true);

            if (approx.size() > 3) {
                // Create a DetectedRegion struct
                DetectedRegion region;


                // Bounding box
                cv::Rect boundingRect = cv::boundingRect(approx);
                cv::RotatedRect rotRect = cv::minAreaRect(approx);
                // cv::Rect boundingRect = 
                
                region.boundingBox = boundingRect;
                region.cropped = scannedImage(boundingRect).clone();

                // cv::polylines(result.annotated, approx, /*isClosed=*/true, 
                //             cv::Scalar(0, 0, 255), /*thickness=*/10);

                // // Draw the bounding rectangle
                // cv::rectangle(result.annotated, boundingRect, cv::Scalar(255, 0, 0), /*thickness=*/10);
                
                cv::Point2f vertices[4];
                rotRect.points(vertices);
                
                std::vector<cv::Point> intCorners;
                for (int i = 0; i < 4; ++i) {
                    intCorners.push_back(cv::Point(vertices[i])); // Implicit conversion to cv::Point
                }

                region.corners = intCorners;



                // Assuming intCorners is a std::vector<cv::Point> with integer points
                for (int i = 0; i < 4; i++) {
                    line(result.annotated, intCorners[i], intCorners[(i + 1) % 4], cv::Scalar(0, 255, 0), 5, cv::LINE_AA);
                }

                // for (int i = 0; i < 4; i++)
                //     line(result.annotated, vertices[i], vertices[(i+1)%4], cv::Scalar(0,255,0), 5, cv::FILLED);

                // Add this region to the result
                result.regions.push_back(std::move(region));
                break;
            }
        }

    }

    return result;
}

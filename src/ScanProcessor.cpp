#include "ScanProcessor.h"
#include <algorithm>
#include <qDebug>

ScanResult ScanProcessor::detectAndCropPhotos(const cv::Mat &scannedImage) {
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
    cv::split(hsv, hsv_channels);         // Split the HSV image into its 3 channels
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
    for (const auto &contour : contours) {
        double contourArea = cv::contourArea(contour);

        if (contourArea > (imgArea / 8)) {
            largeContours.push_back(contour);
        }
    }

    // 5. Approximate each contour and check if it's a quadrilateral

    for (const auto &contour : largeContours) {
        double perimeter = cv::arcLength(contour, true);

        for (int eps = 500; eps > 0; eps--) {
            std::vector<cv::Point> approx;
            cv::approxPolyDP(contour, approx, 0.001 * eps * perimeter, true);

            if (approx.size() > 3) {
                // Create a DetectedRegion struct
                DetectedRegion region;

                // Bounding box
                cv::Rect boundingRect = cv::boundingRect(approx);
                cv::RotatedRect rotRect = cv::minAreaRect(approx);

                region.boundingBox = boundingRect;
                region.cropped = scannedImage(boundingRect).clone();

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

std::vector<cv::Mat> ScanProcessor::cropImages(const cv::Mat &scannedImage,
                                               const std::vector<std::vector<cv::Point>> &quads,
                                               int scannedRotation,
                                               const std::vector<int> &rotations) {
    std::vector<cv::Mat> croppedImages;

    // rotate the image
    cv::Mat rotatedImage;
    cv::Mat rotationMatrix = cv::getRotationMatrix2D(cv::Point(scannedImage.cols / 2, scannedImage.rows / 2), -scannedRotation, 1);
    cv::warpAffine(scannedImage, rotatedImage, rotationMatrix, scannedImage.size(), cv::INTER_LINEAR, cv::BORDER_CONSTANT, cv::Scalar(255, 255, 255)); // Fill border with white

    // Add white padding to the image
    cv::Mat paddedImage;
    int padding = -findMostNegativeXY(quads) * 10;
    int borderType = cv::BORDER_CONSTANT;
    cv::copyMakeBorder(scannedImage, paddedImage, padding, padding, padding, padding,
                       borderType, cv::Scalar(255, 255, 255));

    if (quads.size() != rotations.size()) {
        throw std::invalid_argument("The size of 'quads' and 'rotations' must match.");
    }

    for (size_t i = 0; i < quads.size(); ++i) {
        const auto &quad = quads[i];
        int rotationAngle = (rotations[i] == -1) ? 0 : rotations[i];

        qDebug() << "cropImages: Rotation - " << rotationAngle;

        cv::Mat cropped;
        // Translate quad points by the padding amount
        std::vector<cv::Point> translatedQuad;
        for (const auto &point : quad) {
            translatedQuad.emplace_back(point.x + padding, point.y + padding);
        }

        // Find the rotated rectangle from the translated quad
        cv::RotatedRect rotRect = cv::minAreaRect(translatedQuad);

        cropped = paddedImage(rotRect.boundingRect()).clone();

        // Apply rotation to the cropped region
        if (!cropped.empty() && rotationAngle != 0) {
            // Define the center of rotation
            cv::Point2f center(cropped.cols / 2.0f, cropped.rows / 2.0f);

            // Get the rotation matrix for the given angle
            cv::Mat rotationMatrix = cv::getRotationMatrix2D(center, -rotationAngle, 1.0);

            // Compute the bounding box size of the rotated image
            double absCos = std::abs(rotationMatrix.at<double>(0, 0));
            double absSin = std::abs(rotationMatrix.at<double>(0, 1));
            int newWidth = static_cast<int>(cropped.rows * absSin + cropped.cols * absCos);
            int newHeight = static_cast<int>(cropped.rows * absCos + cropped.cols * absSin);

            // Adjust the rotation matrix to account for translation
            rotationMatrix.at<double>(0, 2) += (newWidth / 2.0 - center.x);
            rotationMatrix.at<double>(1, 2) += (newHeight / 2.0 - center.y);

            // Create a new matrix to store the rotated image with updated size
            cv::Mat rotatedCropped;
            cv::warpAffine(cropped, rotatedCropped, rotationMatrix, cv::Size(newWidth, newHeight),
                           cv::INTER_LINEAR, cv::BORDER_CONSTANT, cv::Scalar(255, 255, 255)); // Fill border with white

            // Update cropped with the rotated version
            cropped = rotatedCropped;
        }

        croppedImages.push_back(cropped);
    }

    return croppedImages;
}

double ScanProcessor::findMostNegativeXY(const std::vector<std::vector<cv::Point>> &quads) {
    double mostNegative = std::numeric_limits<double>::max();

    for (const auto &quad : quads) {
        for (const auto &point : quad) {
            mostNegative = std::min({mostNegative, static_cast<double>(point.x), static_cast<double>(point.y)});
        }
    }

    qDebug() << "Most negative: " << mostNegative;
    if (mostNegative > 0) {
        return -10;
    }

    return mostNegative;
}